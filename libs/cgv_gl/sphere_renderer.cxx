#include "sphere_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {

		render_style* sphere_renderer::create_render_style() const
		{
			return new sphere_render_style();
		}

		sphere_render_style::sphere_render_style() : halo_color(1, 1, 1, 1)
		{
			radius_scale = 1;
			radius = 1;
			use_group_radius = false;

			blend_width_in_pixel = 1.0f;
			halo_width_in_pixel = 0.0f;
			percentual_halo_width = 0.0;
			halo_color_strength = 0.5f;
		}

		sphere_renderer::sphere_renderer()
		{
			has_radii = false;
			has_group_radii = false;
			cull_per_primitive = false;
		}
		///
		void sphere_renderer::set_y_view_angle(float _y_view_angle)
		{
			y_view_angle = _y_view_angle;
		}
		bool sphere_renderer::init(context& ctx)
		{
			bool res = surface_renderer::init(ctx);
			if (!ref_prog().is_created()) {
				if (!ref_prog().build_program(ctx, "sphere.glpr", true)) {
					std::cerr << "ERROR in sphere_renderer::init() ... could not build program sphere.glpr" << std::endl;
					return false;
				}
			}
			return res;
		}
		
		bool sphere_renderer::validate_attributes(const context& ctx) const
		{
			const sphere_render_style& srs = get_style<sphere_render_style>();
			bool res = surface_renderer::validate_attributes(ctx);
			if (!has_group_radii && srs.use_group_radius) {
				ctx.error("sphere_renderer::validate_attributes() group_radii not set");
				res = false;
			}
			return res;
		}
		bool sphere_renderer::enable(context& ctx)
		{
			const sphere_render_style& srs = get_style<sphere_render_style>();

			if (!surface_renderer::enable(ctx))
				return false;

			if (!ref_prog().is_linked())
				return false;
			
			if (!has_radii)
				ref_prog().set_attribute(ctx, "radius", srs.radius);
			
			ref_prog().set_uniform(ctx, "use_group_radius", srs.use_group_radius);
			ref_prog().set_uniform(ctx, "radius_scale", srs.radius_scale);
			float pixel_extent_per_depth = (float)(2.0*tan(0.5*0.0174532925199*y_view_angle) / ctx.get_height());
			ref_prog().set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
			ref_prog().set_uniform(ctx, "blend_width_in_pixel", srs.blend_width_in_pixel);
			ref_prog().set_uniform(ctx, "percentual_halo_width", 0.01f*srs.percentual_halo_width);
			ref_prog().set_uniform(ctx, "halo_width_in_pixel", srs.halo_width_in_pixel);
			ref_prog().set_uniform(ctx, "halo_color", srs.halo_color);
			ref_prog().set_uniform(ctx, "halo_color_strength", srs.halo_color_strength);
			return true;
		}

		bool sphere_renderer::disable(context& ctx)
		{
			const sphere_render_style& srs = get_style<sphere_render_style>();

			if (!attributes_persist()) {
				has_radii = false;
				has_group_radii = false;
			}
			return surface_renderer::disable(ctx);
		}
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			bool sphere_render_style::self_reflect(cgv::reflect::reflection_handler& rh)
			{
				return
					rh.reflect_base(*static_cast<cgv::render::surface_render_style*>(this)) &&
					rh.reflect_member("radius", radius) &&
					rh.reflect_member("use_group_radius", use_group_radius) &&
					rh.reflect_member("radius_scale", radius_scale) &&
					rh.reflect_member("blend_width_in_pixel", blend_width_in_pixel) &&
					rh.reflect_member("halo_width_in_pixel", halo_width_in_pixel) &&
					rh.reflect_member("halo_color", halo_color) &&
					rh.reflect_member("halo_color_strength", halo_color_strength) &&
					rh.reflect_member("percentual_halo_width", percentual_halo_width);
			}

		}
		cgv::reflect::extern_reflection_traits<cgv::render::sphere_render_style, cgv::reflect::render::sphere_render_style> get_reflection_traits(const cgv::render::sphere_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<cgv::render::sphere_render_style, cgv::reflect::render::sphere_render_style>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct sphere_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::sphere_render_style>::get_name())
					return false;
				cgv::render::sphere_render_style* srs_ptr = reinterpret_cast<cgv::render::sphere_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "default radius", srs_ptr->radius, "value_slider", "min=0.01;max=100;log=true;ticks=true");
				p->add_member_control(b, "use_group_radius", srs_ptr->use_group_radius, "check");
				p->add_member_control(b, "radius_scale", srs_ptr->radius_scale, "value_slider", "min=0.01;max=100;log=true;ticks=true");
				bool show = p->begin_tree_node("halo", srs_ptr->halo_color, false, "options='w=120';level=3;align=''");
				p->add_member_control(b, "color", srs_ptr->halo_color, "", "w=50");
				if (show) {
					p->align("\a");
					p->add_member_control(b, "halo_color_strength", srs_ptr->halo_color_strength, "value_slider", "min=0;max=1;ticks=true");
					p->add_member_control(b, "halo_width_in_pixel", srs_ptr->halo_width_in_pixel, "value_slider", "min=-10;max=10;ticks=true");
					p->add_member_control(b, "percentual_halo_width", srs_ptr->percentual_halo_width, "value_slider", "min=-100;max=100;ticks=true");
					p->add_member_control(b, "blend_width_in_pixel", srs_ptr->blend_width_in_pixel, "value_slider", "min=0;max=3;ticks=true");
					p->align("\b");
					p->end_tree_node(srs_ptr->halo_color);
				}
				p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(srs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

		extern CGV_API cgv::gui::gui_creator_registration<sphere_render_style_gui_creator> prs_gc_reg("sphere_render_style_gui_creator");

	}
}