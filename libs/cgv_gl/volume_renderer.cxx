#include "volume_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		volume_renderer& ref_volume_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static volume_renderer r;
			r.manage_singelton(ctx, "volume_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* volume_renderer::create_render_style() const
		{
			return new render_style();
		}

		volume_render_style::volume_render_style()
		{
			transfer_function_texture_unit = 1;
			alpha = 1.0f;
			level_of_detail = 0.0f;
			step_size = 0.002f;
			transformation_matrix = mat4(0.0f);
			transformation_matrix.identity();

			interpolation_mode = IP_LINEAR;
		}

		volume_renderer::volume_renderer()
		{
			shader_defines = "";
			volume_texture = nullptr;
			volume_texture_size = vec3(1.0f);
		}

		bool volume_renderer::validate_attributes(const context& ctx) const
		{
			// validate set attributes
			const volume_render_style& vrs = get_style<volume_render_style>();
			bool res = renderer::validate_attributes(ctx);
			res = res && (volume_texture != nullptr);
			return res;
		}

		bool volume_renderer::init(cgv::render::context& ctx)
		{
			bool res = renderer::init(ctx);
			if (!ref_prog().is_created()) {
				res = res && build_shader(ctx, build_define_string());
			}

			res = res && aa_manager.init(ctx);
			enable_attribute_array_manager(ctx, aa_manager);

			eye_position = vec3(0.0f);
			std::vector<vec3> vertices = {
				vec3(0.0f, 1.0f, 0.0f),
				vec3(1.0f, 1.0f, 0.0f),
				vec3(0.0f, 0.0f, 0.0f),
				vec3(1.0f, 0.0f, 0.0f),
				vec3(1.0f, 0.0f, 1.0f),
				vec3(1.0f, 1.0f, 0.0f),
				vec3(1.0f, 1.0f, 1.0f),
				vec3(0.0f, 1.0f, 0.0f),
				vec3(0.0f, 1.0f, 1.0f),
				vec3(0.0f, 0.0f, 0.0f),
				vec3(0.0f, 0.0f, 1.0f),
				vec3(1.0f, 0.0f, 1.0f),
				vec3(0.0f, 1.0f, 1.0f),
				vec3(1.0f, 1.0f, 1.0f)
			};
			set_position_array(ctx, vertices);

			return res;
		}

		bool volume_renderer::set_volume_texture(texture* _volume_texture)
		{
			if(!_volume_texture)
				return false;
			if(_volume_texture->get_nr_dimensions() != 3)
				return false;

			volume_texture = _volume_texture;
			volume_texture_size = vec3(float(volume_texture->get_width()), float(volume_texture->get_height()), float(volume_texture->get_depth()));
			return true;
		}

		void volume_renderer::set_eye_position(vec3 _eye_position)
		{
			eye_position = _eye_position;
		}

		std::string volume_renderer::build_define_string()
		{
			const volume_render_style& vrs = get_style<volume_render_style>();

			std::string defines = "INTERPOLATION_MODE=";
			defines += std::to_string((int)vrs.interpolation_mode);
			return defines;
		}

		bool volume_renderer::build_shader(context& ctx, std::string defines)
		{
			shader_defines = defines;
			if(ref_prog().is_created())
				ref_prog().destruct(ctx);

			if(!ref_prog().is_created()) {
				if(!ref_prog().build_program(ctx, "volume.glpr", true, defines)) {
					std::cerr << "ERROR in volume_renderer::init() ... could not build program volume.glpr" << std::endl;
					return false;
				}
			}
			return true;
		}

		bool volume_renderer::enable(context& ctx)
		{
			const volume_render_style& vrs = get_style<volume_render_style>();

			std::string defines = build_define_string();
			if(defines != shader_defines) {
				if(!build_shader(ctx, defines))
					return false;
			}

			if (!renderer::enable(ctx))
				return false;
			ref_prog().set_uniform(ctx, "volume_tex", 0);
			ref_prog().set_uniform(ctx, "transfer_function_tex", vrs.transfer_function_texture_unit);
			ref_prog().set_uniform(ctx, "eye_position", eye_position);
			ref_prog().set_uniform(ctx, "alpha_coeff", vrs.alpha);
			ref_prog().set_uniform(ctx, "lod", vrs.level_of_detail);
			ref_prog().set_uniform(ctx, "step_size", vrs.step_size);
			ref_prog().set_uniform(ctx, "tex_size", volume_texture_size);
			ref_prog().set_uniform(ctx, "tex_coord_scaling", vec3(cgv::math::max_value(volume_texture_size)) / volume_texture_size);
			ref_prog().set_uniform(ctx, "transformation_matrix", vrs.transformation_matrix);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			volume_texture->enable(ctx, 0);
			return true;
		}
		///
		bool volume_renderer::disable(context& ctx)
		{
			volume_texture->disable(ctx);

			glDisable(GL_BLEND);
			glDisable(GL_CULL_FACE);

			if (!attributes_persist()) {}

			return renderer::disable(ctx);
		}
		///
		void volume_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)14);
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

				p->add_member_control(b, "alpha", vrs_ptr->alpha, "value_slider", "min=0.0;step=0.001;max=1.0;ticks=true");
				p->add_member_control(b, "lod", vrs_ptr->level_of_detail, "value_slider", "min=0.0;max=9.0;ticks=true");
				p->add_member_control(b, "step_size", vrs_ptr->step_size, "value_slider", "min=0.0005;step=0.00001;max=0.01;log=true;ticks=true");
				p->add_member_control(b, "interpolation", vrs_ptr->interpolation_mode, "dropdown", "enums=nearest,linear,smooth,cubic");
				return true;
			}
		};

#include "gl/lib_begin.h"

		CGV_API cgv::gui::gui_creator_registration<volume_render_style_gui_creator> volume_rs_gc_reg("volume_render_style_gui_creator");

	}
}
