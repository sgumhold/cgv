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
			enable_noise_offset = true;
			interpolation_mode = IP_LINEAR;
			enable_depth_test = true;

			compositing_mode = CM_BLEND;

			scale_adjustment_factor = 100.0f;

			enable_lighting = false;
			light_local_to_eye = true;
			use_gradient_texture = false;
			light_direction = normalize(vec3(-1.0f, 1.0f, 1.0f));
			ambient_strength = 0.3f;
			diffuse_strength = 0.8f;
			specular_strength = 0.4f;
			roughness = 0.3f;
			specular_color_mix = 0.0f;

			enable_gradient_modulation = false;
			gradient_lambda = 0.0f;

			isosurface_mode = IM_NONE;
			isovalue = 0.5f;
			isosurface_color = rgb(0.7f);
			isosurface_color_from_transfer_function = false;

			slice_mode = SM_DISABLED;
			slice_axis = 2;
			slice_coordinate = 0.5f;
			slice_opacity = 0.5f;

			clip_box = box3(vec3(0.0f), vec3(1.0f));
		}

		volume_renderer::volume_renderer() : noise_texture("uint8[R]")
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

			noise_offset = vec2(0.0f);
		}

		void volume_renderer::init_noise_texture(context& ctx)
		{
			if(noise_texture.is_created())
				noise_texture.destruct(ctx);

			unsigned size = 64;
			std::vector<uint8_t> noise_data(size*size);

			std::default_random_engine rng(42);
			std::uniform_int_distribution<unsigned> dist(0u, 255u);

			for(size_t i = 0; i < noise_data.size(); ++i)
				noise_data[i] = static_cast<uint8_t>(dist(rng));

			cgv::data::data_view dv = cgv::data::data_view(new cgv::data::data_format(size, size, TI_UINT8, cgv::data::CF_R), noise_data.data());
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
			shader_code::set_define(defines, "ENABLE_LIGHTING", vrs.enable_lighting, false);
			shader_code::set_define(defines, "USE_GRADIENT_TEXTURE", vrs.use_gradient_texture, false);
			shader_code::set_define(defines, "ENABLE_GRADIENT_MODULATION", vrs.enable_gradient_modulation, false);
			shader_code::set_define(defines, "ENABLE_DEPTH_TEST", vrs.enable_depth_test, false);
			
			shader_code::set_define(defines, "ISOSURFACE_MODE", vrs.isosurface_mode, volume_render_style::IM_NONE);
			shader_code::set_define(defines, "ISOSURFACE_COLOR_MODE", vrs.isosurface_color_from_transfer_function, false);

			shader_code::set_define(defines, "SLICE_MODE", vrs.slice_mode, volume_render_style::SM_DISABLED);

			shader_code::set_define(defines, "COMPOSITING_MODE", vrs.compositing_mode, volume_render_style::CM_BLEND);
			if(transfer_function_texture)
				shader_code::set_define(defines, "TRANSFER_FUNCTION_SAMPLER_DIMENSIONS", transfer_function_texture->get_nr_dimensions(), 1u);
		}
		bool volume_renderer::init(cgv::render::context& ctx)
		{
			bool res = renderer::init(ctx);
			res = res && position_aam.init(ctx);
			enable_attribute_array_manager(ctx, position_aam);

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
		
		void volume_renderer::set_noise_offset(const vec2& offset)
		{
			noise_offset = offset;
		}

		bool volume_renderer::enable(context& ctx) {
			const volume_render_style& vrs = get_style<volume_render_style>();
			if(!renderer::enable(ctx))
				return false;
			int vp[4];
			glGetIntegerv(GL_VIEWPORT, vp);
			ref_prog().set_uniform(ctx, "viewport_dims", vec2(float(vp[2] - vp[0]), float(vp[3] - vp[1])));
			ref_prog().set_uniform(ctx, "noise_offset", noise_offset);

			ref_prog().set_uniform(ctx, "scale_adjustment_factor", vrs.scale_adjustment_factor);

			ref_prog().set_uniform(ctx, "light_local_to_eye", vrs.light_local_to_eye);
			ref_prog().set_uniform(ctx, "light_direction", normalize(vrs.light_direction));
			float specular_exponent = cgv::math::lerp(128.0f, 1.0f, cgv::math::clamp(vrs.roughness, 0.0f, 1.0f));
			ref_prog().set_uniform(ctx, "ambient_strength", vrs.ambient_strength);
			ref_prog().set_uniform(ctx, "diffuse_strength", vrs.diffuse_strength);
			ref_prog().set_uniform(ctx, "specular_strength", vrs.specular_strength);
			ref_prog().set_uniform(ctx, "specular_exponent", specular_exponent);
			ref_prog().set_uniform(ctx, "specular_color_mix", vrs.specular_color_mix);

			ref_prog().set_uniform(ctx, "gradient_lambda", std::max(vrs.gradient_lambda, 0.001f));

			ref_prog().set_uniform(ctx, "isovalue", vrs.isovalue);
			ref_prog().set_uniform(ctx, "isosurface_color", vrs.isosurface_color);

			ref_prog().set_uniform(ctx, "slice_axis", vrs.slice_axis);
			ref_prog().set_uniform(ctx, "slice_coordinate", vrs.slice_coordinate);
			ref_prog().set_uniform(ctx, "slice_opacity", vrs.slice_opacity);

			ref_prog().set_uniform(ctx, "clip_box_min", vrs.clip_box.get_min_pnt());

			ref_prog().set_uniform(ctx, "clip_box_max", vrs.clip_box.get_max_pnt());

			ctx.push_depth_test_state();
			ctx.disable_depth_test();
			ctx.push_blend_state();
			ctx.enable_blending();
			ctx.set_blend_func_back_to_front();
			ctx.push_cull_state();
			ctx.set_cull_state(CM_FRONTFACE);
			
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

			ctx.pop_cull_state();
			ctx.pop_blend_state();
			ctx.pop_depth_test_state();
			
			return renderer::disable(ctx);
		}
		///
		void volume_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			ctx.push_modelview_matrix();

			if(apply_bounding_box_transformation) {
				ctx.mul_modelview_matrix(cgv::math::translate4(bounding_box.get_center()));
				ctx.mul_modelview_matrix(cgv::math::scale4(bounding_box.get_extent()));
			}

			glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)14);

			ctx.pop_modelview_matrix();
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

				p->add_member_control(b, "Quality", vrs_ptr->integration_quality, "dropdown", "w=114;enums='8=8,16=16,32=32,64=64,128=128,256=256,512=512,1024=1024,2048=2048,4096=4096'", " ");
				p->add_member_control(b, "Use Noise", vrs_ptr->enable_noise_offset, "check", "w=74");
				p->add_member_control(b, "Interpolation", vrs_ptr->interpolation_mode, "dropdown", "enums=Nearest,Smoothed,Linear,Cubic");

				p->add_member_control(b, "Depth Test", vrs_ptr->enable_depth_test, "check");

				p->add_member_control(b, "Compositing Mode", vrs_ptr->compositing_mode, "dropdown", "enums='Maximum Intensity Projection, Average, Blend'");

				p->add_member_control(b, "Scale Adjustment", vrs_ptr->scale_adjustment_factor, "value_slider", "min=0.0;step=0.001;max=1000.0;log=true;ticks=true");
				
				if(p->begin_tree_node("Lighting", vrs_ptr->enable_lighting, false)) {
					p->align("/a");
					p->add_member_control(b, "Enable", vrs_ptr->enable_lighting, "check", "w=88", " ");
					p->add_member_control(b, "Local to Eye", vrs_ptr->light_local_to_eye, "check", "w=100");
					p->add_member_control(b, "Normals from Gradient Texture", vrs_ptr->use_gradient_texture, "check");
					p->add_member_control(b, "Direction", vrs_ptr->light_direction[0], "value", "w=58;min=-1;max=1", " ");
					p->add_member_control(b, "", vrs_ptr->light_direction[1], "value", "w=58;min=-1;max=1", " ");
					p->add_member_control(b, "", vrs_ptr->light_direction[2], "value", "w=58;min=-1;max=1");
					p->add_member_control(b, "", vrs_ptr->light_direction[0], "slider", "w=58;min=-1;max=1;step=0.0001;ticks=true", " ");
					p->add_member_control(b, "", vrs_ptr->light_direction[1], "slider", "w=58;min=-1;max=1;step=0.0001;ticks=true", " ");
					p->add_member_control(b, "", vrs_ptr->light_direction[2], "slider", "w=58;min=-1;max=1;step=0.0001;ticks=true");

					p->add_decorator("Light Parameters", "heading", "level=3");
					p->add_member_control(b, "Ambient", vrs_ptr->ambient_strength, "value_slider", "min=0;max=1;step=0.001;ticks=true");
					p->add_member_control(b, "Diffuse", vrs_ptr->diffuse_strength, "value_slider", "min=0;max=1;step=0.001;ticks=true");
					p->add_member_control(b, "Specular", vrs_ptr->specular_strength, "value_slider", "min=0;max=1;step=0.001;ticks=true");
					p->add_member_control(b, "Roughness", vrs_ptr->roughness, "value_slider", "min=0;max=1;step=0.001;ticks=true");
					p->add_member_control(b, "Specular Color", vrs_ptr->specular_color_mix, "value_slider", "min=0;max=1;step=0.001;ticks=true");
					p->align("/b");
					p->end_tree_node(vrs_ptr->enable_lighting);
				}

				if(p->begin_tree_node("Gradient Modulation", vrs_ptr->enable_gradient_modulation, false)) {
					p->align("\a");
					p->add_member_control(b, "Enable", vrs_ptr->enable_gradient_modulation, "check");
					p->add_member_control(b, "Lambda", vrs_ptr->gradient_lambda, "value_slider", "min=0.001;max=10;step=0.001;ticks=true;log=true");
					p->align("\b");
					p->end_tree_node(vrs_ptr->enable_gradient_modulation);
				}
				if (p->begin_tree_node("Orthogonal Slicing", vrs_ptr->slice_mode, false)) {
					p->align("\a");
					p->add_member_control(b, "Mode", vrs_ptr->slice_mode, "dropdown", "enums='Disabled,Opaque,Transparent'");
					p->add_member_control(b, "Axis", vrs_ptr->slice_axis, "value_slider", "min=0;max=2;ticks=true");
					p->add_member_control(b, "Coordinate", vrs_ptr->slice_coordinate, "value_slider", "min=0;max=1;ticks=true");
					p->add_member_control(b, "Opacity", vrs_ptr->slice_opacity, "value_slider", "min=0;max=1;ticks=true");
					p->align("\b");
					p->end_tree_node(vrs_ptr->slice_mode);
				}

				if(p->begin_tree_node("Isosurface (Blend only)", vrs_ptr->isosurface_mode, false)) {
					p->align("\a");
					p->add_member_control(b, "", vrs_ptr->isosurface_mode, "dropdown", "enums='Disabled,Isovalue,Alpha Threshold'");

					p->add_member_control(b, "Value", vrs_ptr->isovalue, "value_slider", "min=0.0;max=1.0;step=0.001;ticks=true");
					p->add_member_control(b, "Color", vrs_ptr->isosurface_color, "", "w=42", " ");
					p->add_member_control(b, "From Transfer Function", vrs_ptr->isosurface_color_from_transfer_function, "check", "w=146");
					p->align("\b");
					p->end_tree_node(vrs_ptr->isosurface_mode);
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
					p->end_tree_node(vrs_ptr->clip_box);
				}

				return true;
			}
		};

#include "gl/lib_begin.h"

		CGV_API cgv::gui::gui_creator_registration<volume_render_style_gui_creator> volume_rs_gc_reg("volume_render_style_gui_creator");

	}
}
