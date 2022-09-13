#include "volume_renderer.h"
#include <random>
#include <cgv/math/ftransform.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		volume_renderer& ref_volume_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static volume_renderer r;
			r.manage_singleton(ctx, "volume_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* volume_renderer::create_render_style() const
		{
			return new volume_render_style();
		}

		volume_render_style::volume_render_style()
		{
			integration_quality = IQ_128;
			interpolation_mode = IP_LINEAR;
			enable_noise_offset = true;
			opacity_scale = 1.0f;
			enable_scale_adjustment = true;
			size_scale = 100.0f;
			compositing_mode = CM_BLEND;
			clip_box = box3(vec3(0.0f), vec3(1.0f));
			enable_lighting = false;
			enable_depth_test = true;
			front_to_back = true;
			
			light_static_to_scene = true;
			light_direction = normalize(vec3(1.0f, -1.0f, -1.0f));

			diffuse_strength = 0.8f;
			specular_strength = 0.4f;
			specular_power = 32.0f;
			ambient_strength = 0.3f;

			isosurface_mode = IM_NONE;
			isosurface_color_from_transfer_function = false;
			isovalue = 0.5f;
			isosurface_color = rgb(0.7f);
		}

		volume_renderer::volume_renderer() : noise_texture("flt32[R]")
		{
			volume_texture = nullptr;
			transfer_function_texture = nullptr;
			gradient_texture = nullptr;
			depth_texture = nullptr;

			noise_texture.set_min_filter(TF_LINEAR);
			noise_texture.set_mag_filter(TF_LINEAR);
			noise_texture.set_wrap_s(TW_REPEAT);
			noise_texture.set_wrap_t(TW_REPEAT);

			bounding_box = box3(vec3(0.0f), vec3(1.0f));
			apply_bounding_box_transformation = false;
		}

		void volume_renderer::init_noise_texture(context& ctx)
		{
			if(noise_texture.is_created())
				noise_texture.destruct(ctx);

			unsigned size = 64;
			std::vector<float> noise_data(size*size);

			std::default_random_engine rng(42);
			std::uniform_real_distribution<float> dist(0.0f, 1.0f);

			for(size_t i = 0; i < noise_data.size(); ++i)
				noise_data[i] = dist(rng);

			cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(size, size, TI_FLT32, cgv::data::CF_R), noise_data.data());
			noise_texture.create(ctx, dv, 0);
		}

		bool volume_renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			const volume_render_style& vrs = get_style<volume_render_style>();
			bool res = renderer::validate_attributes(ctx);
			res = res && (volume_texture != nullptr);
			return res;
		}
		bool volume_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "volume.glpr", true, defines);
		}
		void volume_renderer::update_defines(shader_define_map& defines)
		{
			const volume_render_style& vrs = get_style<volume_render_style>();

			shader_code::set_define(defines, "NUM_STEPS", vrs.integration_quality, volume_render_style::IQ_128);
			shader_code::set_define(defines, "INTERPOLATION_MODE", vrs.interpolation_mode, volume_render_style::IP_LINEAR);
			shader_code::set_define(defines, "ENABLE_NOISE_OFFSET", vrs.enable_noise_offset, true);
			shader_code::set_define(defines, "ENABLE_SCALE_ADJUSTMENT", vrs.enable_scale_adjustment, false);
			shader_code::set_define(defines, "ENABLE_LIGHTING", vrs.enable_lighting, false);
			shader_code::set_define(defines, "ENABLE_DEPTH_TEST", vrs.enable_depth_test, false);
			
			shader_code::set_define(defines, "FRONT_TO_BACK", vrs.front_to_back || (vrs.isosurface_mode != volume_render_style::IM_NONE && vrs.compositing_mode == volume_render_style::CM_BLEND), false);
			shader_code::set_define(defines, "ISOSURFACE_MODE", vrs.isosurface_mode, volume_render_style::IM_NONE);
			shader_code::set_define(defines, "ISOSURFACE_COLOR_MODE", vrs.isosurface_color_from_transfer_function, false);

			shader_code::set_define(defines, "COMPOSITING_MODE", vrs.compositing_mode, volume_render_style::CM_BLEND);
			if(transfer_function_texture)
				shader_code::set_define(defines, "TRANSFER_FUNCTION_SAMPLER_DIMENSIONS", transfer_function_texture->get_nr_dimensions(), 1u);
		}
		bool volume_renderer::init(cgv::render::context& ctx)
		{
			bool res = renderer::init(ctx);
			// TOOD: use aam from renderer base class
			res = res && aa_manager.init(ctx);
			enable_attribute_array_manager(ctx, aa_manager);

			// use a single optimized triangle strip to define a cube
			std::vector<vec3> positions = {
				vec3(-0.5f, +0.5f, -0.5f),
				vec3(+0.5f, +0.5f, -0.5f),
				vec3(-0.5f, -0.5f, -0.5f),
				vec3(+0.5f, -0.5f, -0.5f),
				vec3(+0.5f, -0.5f, +0.5f),
				vec3(+0.5f, +0.5f, -0.5f),
				vec3(+0.5f, +0.5f, +0.5f),
				vec3(-0.5f, +0.5f, -0.5f),
				vec3(-0.5f, +0.5f, +0.5f),
				vec3(-0.5f, -0.5f, -0.5f),
				vec3(-0.5f, -0.5f, +0.5f),
				vec3(+0.5f, -0.5f, +0.5f),
				vec3(-0.5f, +0.5f, +0.5f),
				vec3(+0.5f, +0.5f, +0.5f)
			};
			set_position_array(ctx, positions);
			
			if(!noise_texture.is_created())
				init_noise_texture(ctx);

			return res;
		}

		bool volume_renderer::set_volume_texture(texture* tex) {
			if(!tex || tex->get_nr_dimensions() != 3)
				return false;
			volume_texture = tex;
			return true;
		}

		bool volume_renderer::set_transfer_function_texture(texture* tex) {
			if(!tex || tex->get_nr_dimensions() > 2)
				return false;
			transfer_function_texture = tex;
			return true;
		}

		bool volume_renderer::set_gradient_texture(texture* tex) {
			if(!tex || tex->get_nr_dimensions() != 3)
				return false;
			gradient_texture = tex;
			return true;
		}

		bool volume_renderer::set_depth_texture(texture* tex) {
			if(!tex || tex->get_nr_dimensions() != 2)
				return false;
			depth_texture = tex;
			return true;
		}
		
		void volume_renderer::set_bounding_box(const box3& bbox)
		{
			bounding_box = bbox;
		}
		
		void volume_renderer::transform_to_bounding_box(bool flag)
		{
			apply_bounding_box_transformation = flag;
		}

		bool volume_renderer::enable(context& ctx)
		{
			const volume_render_style& vrs = get_style<volume_render_style>();
			if (!renderer::enable(ctx))
				return false;
			int vp[4];
			glGetIntegerv(GL_VIEWPORT, vp);
			ref_prog().set_uniform(ctx, "viewport_dims", vec2(float(vp[2]-vp[0]), float(vp[3]-vp[1])));
			ref_prog().set_uniform(ctx, "opacity_scale", vrs.opacity_scale);
			ref_prog().set_uniform(ctx, "size_scale", vrs.size_scale);
			ref_prog().set_uniform(ctx, "clip_box_min", vrs.clip_box.get_min_pnt());
			ref_prog().set_uniform(ctx, "clip_box_max", vrs.clip_box.get_max_pnt());

			ref_prog().set_uniform(ctx, "light_static_to_scene", vrs.light_static_to_scene);
			ref_prog().set_uniform(ctx, "light_direction", normalize(vrs.light_direction));
			ref_prog().set_uniform(ctx, "light_parameters", vec4(vrs.diffuse_strength, vrs.specular_strength, vrs.specular_power, vrs.ambient_strength));

			ref_prog().set_uniform(ctx, "isovalue", vrs.isovalue);
			ref_prog().set_uniform(ctx, "isosurface_color", vrs.isosurface_color);

			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);

			if(volume_texture) volume_texture->enable(ctx, 0);
			if(transfer_function_texture) transfer_function_texture->enable(ctx, 1);
			noise_texture.enable(ctx, 2);
			if(gradient_texture) gradient_texture->enable(ctx, 3);
			if(depth_texture) depth_texture->enable(ctx, 4);
			return true;
		}
		///
		bool volume_renderer::disable(context& ctx)
		{
			if(volume_texture) volume_texture->disable(ctx);
			if(transfer_function_texture) transfer_function_texture->disable(ctx);
			noise_texture.disable(ctx);
			if(gradient_texture) gradient_texture->disable(ctx);
			if(depth_texture) depth_texture->disable(ctx);

			glDisable(GL_CULL_FACE);
			glDisable(GL_BLEND);

			return renderer::disable(ctx);
		}
		///
		void volume_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			if(apply_bounding_box_transformation) {
				mat4 scale = cgv::math::scale4(bounding_box.get_extent());
				mat4 translation = cgv::math::translate4(bounding_box.get_center());

				ctx.push_modelview_matrix();
				ctx.mul_modelview_matrix(translation);
				ctx.mul_modelview_matrix(scale);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)14);

				ctx.pop_modelview_matrix();
			} else {
				glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)14);
			}
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct volume_render_style_gui_creator : public gui_creator {
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*) {
				if(value_type != cgv::type::info::type_name<cgv::render::volume_render_style>::get_name())
					return false;
				cgv::render::volume_render_style* vrs_ptr = reinterpret_cast<cgv::render::volume_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "Quality", vrs_ptr->integration_quality, "dropdown", "w=114;enums='8=8,16=16,32=32,64=64,128=128,256=256,512=512,1024=1024'", " ");
				p->add_member_control(b, "Use Noise", vrs_ptr->enable_noise_offset, "check", "w=74");
				p->add_member_control(b, "Interpolation", vrs_ptr->interpolation_mode, "dropdown", "enums=Nearest,Linear,Smooth,Cubic");

				p->add_member_control(b, "Compositing Mode", vrs_ptr->compositing_mode, "dropdown", "enums='Maximum Intensity Projection, Average, Blend'");
				p->add_member_control(b, "Front-to-Back", vrs_ptr->front_to_back, "check");

				p->add_member_control(b, "Scale Adjustment", vrs_ptr->size_scale, "value_slider", "w=170;min=0.0;step=0.001;max=500.0;log=true;ticks=true", " ");
				p->add_member_control(b, "", vrs_ptr->enable_scale_adjustment, "check", "w=30");
				p->add_member_control(b, "Opacity Scale", vrs_ptr->opacity_scale, "value_slider", "min=0.0;step=0.001;max=1.0;ticks=true");
				
				p->add_member_control(b, "Depth Test", vrs_ptr->enable_depth_test, "check");
	
				if(p->begin_tree_node("Lighting", vrs_ptr->enable_lighting, false)) {
					p->align("/a");
					p->add_member_control(b, "Enable", vrs_ptr->enable_lighting, "check", "w=88", " ");
					p->add_member_control(b, "Static to Scene", vrs_ptr->light_static_to_scene, "check", "w=100");
					p->add_member_control(b, "Direction", vrs_ptr->light_direction[0], "value", "w=58;min=-1;max=1", " ");
					p->add_member_control(b, "", vrs_ptr->light_direction[1], "value", "w=58;min=-1;max=1", " ");
					p->add_member_control(b, "", vrs_ptr->light_direction[2], "value", "w=58;min=-1;max=1");
					p->add_member_control(b, "", vrs_ptr->light_direction[0], "slider", "w=58;min=-1;max=1;step=0.0001;ticks=true", " ");
					p->add_member_control(b, "", vrs_ptr->light_direction[1], "slider", "w=58;min=-1;max=1;step=0.0001;ticks=true", " ");
					p->add_member_control(b, "", vrs_ptr->light_direction[2], "slider", "w=58;min=-1;max=1;step=0.0001;ticks=true");

					p->add_decorator("Light Parameters", "heading", "level=3");
					p->add_member_control(b, "Diffuse", vrs_ptr->diffuse_strength, "value_slider", "min=0;max=1;step=0.001;ticks=true");
					p->add_member_control(b, "Specular", vrs_ptr->specular_strength, "value_slider", "min=0;max=1;step=0.001;ticks=true");
					p->add_member_control(b, "Specular Power", vrs_ptr->specular_power, "value_slider", "min=1;max=128;step=0.1;ticks=true;log=true");
					p->add_member_control(b, "Ambient", vrs_ptr->ambient_strength, "value_slider", "min=0;max=1;step=0.001;ticks=true");
					p->align("/b");
				}

				if(p->begin_tree_node("Isosurface (Blend only)", vrs_ptr->isosurface_mode, false)) {
					p->align("\a");
					p->add_member_control(b, "", vrs_ptr->isosurface_mode, "dropdown", "enums='Disabled,Isovalue,Alpha Threshold'");

					p->add_member_control(b, "Value", vrs_ptr->isovalue, "value_slider", "min=0.0;max=1.0;step=0.001;ticks=true");
					p->add_member_control(b, "Color", vrs_ptr->isosurface_color, "", "w=42", " ");
					p->add_member_control(b, "From Transfer Function", vrs_ptr->isosurface_color_from_transfer_function, "check");
					p->align("\b");
				}
				
				if(p->begin_tree_node("Clip Box", vrs_ptr->clip_box, false)) {
					p->align("\a");
					p->add_member_control(b, "Box Min", vrs_ptr->clip_box.ref_min_pnt()[0], "value", "w=58;min=0;max=1", " ");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_min_pnt()[1], "value", "w=58;min=0;max=1", " ");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_min_pnt()[2], "value", "w=58;min=0;max=1");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_min_pnt()[0], "slider", "w=58;min=0;max=1;step=0.0001;ticks=true", " ");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_min_pnt()[1], "slider", "w=58;min=0;max=1;step=0.0001;ticks=true", " ");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_min_pnt()[2], "slider", "w=58;min=0;max=1;step=0.0001;ticks=true");

					p->add_member_control(b, "Box Max", vrs_ptr->clip_box.ref_max_pnt()[0], "value", "w=58;max=0;max=1", " ");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_max_pnt()[1], "value", "w=58;max=0;max=1", " ");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_max_pnt()[2], "value", "w=58;max=0;max=1");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_max_pnt()[0], "slider", "w=58;max=0;max=1;step=0.0001;ticks=true", " ");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_max_pnt()[1], "slider", "w=58;max=0;max=1;step=0.0001;ticks=true", " ");
					p->add_member_control(b, "", vrs_ptr->clip_box.ref_max_pnt()[2], "slider", "w=58;max=0;max=1;step=0.0001;ticks=true");
					p->align("\b");
				}

				return true;
			}
		};

#include "gl/lib_begin.h"

		CGV_API cgv::gui::gui_creator_registration<volume_render_style_gui_creator> volume_rs_gc_reg("volume_render_style_gui_creator");

	}
}
