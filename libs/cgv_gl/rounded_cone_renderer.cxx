#include "rounded_cone_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		rounded_cone_renderer& ref_rounded_cone_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static rounded_cone_renderer r;
			r.manage_singelton(ctx, "rounded_cone_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* rounded_cone_renderer::create_render_style() const
		{
			return new surface_render_style();
		}

		rounded_cone_render_style::rounded_cone_render_style()
		{
			radius = 1.0f;
			radius_scale = 1.0f;
			
			ao_offset = 0.04f;
			ao_distance = 0.8f;
			ao_strength = 1.0f;

			tex_offset = vec3(0.0f);
			tex_scaling = vec3(1.0f);
			tex_coord_scaling = vec3(1.0f);
			texel_size = 1.0f;

			sample_dirs.push_back(vec3(1.0f, 0.0f, 0.0f));
			sample_dirs.push_back(vec3(0.0f, 1.0f, 0.0f));
			sample_dirs.push_back(vec3(0.0f, 0.0f, 1.0f));
		}

		rounded_cone_renderer::rounded_cone_renderer()
		{
			has_radii = false;
		}

		void rounded_cone_renderer::set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr)
		{
			surface_renderer::set_attribute_array_manager(ctx, _aam_ptr);
			if(aam_ptr) {
				if(aam_ptr->has_attribute(ctx, ref_prog().get_attribute_location(ctx, "radius")))
					has_radii = true;
			} else {
				has_radii = false;
			}
		}

		bool rounded_cone_renderer::validate_attributes(const context& ctx) const
		{
			const rounded_cone_render_style& rcrs = get_style<rounded_cone_render_style>();
			bool res = surface_renderer::validate_attributes(ctx);
			return res;
		}
		bool rounded_cone_renderer::init(cgv::render::context& ctx)
		{
			bool res = renderer::init(ctx);
			if (!ref_prog().is_created()) {
				if (!ref_prog().build_program(ctx, "rounded_cone.glpr", true)) {
					std::cerr << "ERROR in rounded_cone_renderer::init() ... could not build program rounded_cone.glpr" << std::endl;
					return false;
				}
			}
			return res;
		}

		/// 
		bool rounded_cone_renderer::enable(context& ctx)
		{
			const rounded_cone_render_style& rcrs = get_style<rounded_cone_render_style>();

			if (!surface_renderer::enable(ctx))
				return false;

			if(!ref_prog().is_linked())
				return false;

			if(!has_radii)
				ref_prog().set_attribute(ctx, "radius", rcrs.radius);

			ref_prog().set_uniform(ctx, "radius_scale", rcrs.radius_scale);
			ref_prog().set_uniform(ctx, "ao_offset", rcrs.ao_offset);
			ref_prog().set_uniform(ctx, "ao_distance", rcrs.ao_distance);
			ref_prog().set_uniform(ctx, "ao_strength", rcrs.ao_strength);
			ref_prog().set_uniform(ctx, "density_tex_offset", rcrs.tex_offset);
			ref_prog().set_uniform(ctx, "density_tex_scaling", rcrs.tex_scaling);
			ref_prog().set_uniform(ctx, "tex_coord_scaling", rcrs.tex_coord_scaling);
			ref_prog().set_uniform(ctx, "texel_size", rcrs.texel_size);
			ref_prog().set_uniform(ctx, "cone_angle_factor", rcrs.cone_angle_factor);			
			ref_prog().set_uniform_array(ctx, "sample_dirs", rcrs.sample_dirs);
			return true;
		}
		///
		bool rounded_cone_renderer::disable(context& ctx)
		{
			if(!attributes_persist()) {
				has_radii = false;
			}
			return surface_renderer::disable(ctx);
		}

		bool rounded_cone_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<rounded_cone_render_style*>(this)) &&
				rh.reflect_member("radius", radius);
		}

		cgv::reflect::extern_reflection_traits<rounded_cone_render_style, rounded_cone_render_style_reflect> get_reflection_traits(const rounded_cone_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<rounded_cone_render_style, rounded_cone_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct rounded_cone_render_style_gui_creator : public gui_creator {
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*) {
				if(value_type != cgv::type::info::type_name<cgv::render::rounded_cone_render_style>::get_name())
					return false;
				cgv::render::rounded_cone_render_style* rcrs_ptr = reinterpret_cast<cgv::render::rounded_cone_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "default radius", rcrs_ptr->radius, "value_slider", "min=0.001;step=0.0001;max=10.0;log=true;ticks=true");
				p->add_member_control(b, "radius scale", rcrs_ptr->radius_scale, "value_slider", "min=0.01;step=0.0001;max=100.0;log=true;ticks=true");
				
				// ambient occlusion
				p->add_decorator("ambient occlusion", "heading", "level=4");
				p->add_member_control(b, "ao offset", rcrs_ptr->ao_offset, "value_slider", "min=0.0;step=0.0001;max=0.2;log=true;ticks=true");
				p->add_member_control(b, "ao distance", rcrs_ptr->ao_distance, "value_slider", "min=0.0;step=0.0001;max=1.0;log=true;ticks=true");
				p->add_member_control(b, "ao strength", rcrs_ptr->ao_strength, "value_slider", "min=0.0;step=0.0001;max=10.0;log=true;ticks=true");

				p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(rcrs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

		CGV_API cgv::gui::gui_creator_registration<rounded_cone_render_style_gui_creator> rounded_cone_rs_gc_reg("rounded_cone_render_style_gui_creator");

	}
}
