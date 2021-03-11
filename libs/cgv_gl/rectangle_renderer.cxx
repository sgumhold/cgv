#include "rectangle_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		rectangle_renderer& ref_rectangle_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static rectangle_renderer r;
			r.manage_singelton(ctx, "rectangle_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* rectangle_renderer::create_render_style() const
		{
			return new rectangle_render_style();
		}

		rectangle_render_style::rectangle_render_style()
		{
			border_mode = 3;
			pixel_blend = 1.0f;
			percentual_border_width = 0.0f;
			border_width_in_pixel = 0.0f;
			default_depth_offset = 0.0f;
			texture_mode = 0;
			border_color = rgba(0.0f,0.0f,0.0f,1.0f);
		}

		rectangle_renderer::rectangle_renderer() {
 			has_extents = false;
			has_translations = false;
			has_rotations = false;
			has_texcoords = false;
			position_is_center = true;
			has_depth_offsets = false;
			y_view_angle = 45;
		}
		///
		void rectangle_renderer::set_y_view_angle(float _y_view_angle)
		{
			y_view_angle = _y_view_angle;
		}

		/// call this before setting attribute arrays to manage attribute array in given manager
		void rectangle_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "extent"))
				has_extents = true;
			if (has_attribute(ctx, "translation"))
				has_translations = true;
			if (has_attribute(ctx, "rotation"))
				has_rotations = true;
			if (has_attribute(ctx, "texcoord"))
				has_texcoords = true;
			if (has_attribute(ctx, "depth_offset"))
				has_depth_offsets = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void rectangle_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::disable_attribute_array_manager(ctx, aam);
			has_extents = false;
			has_translations = false;
			has_rotations = false;
			has_texcoords = false;
			has_depth_offsets = false;
		}
		
		bool rectangle_renderer::init(context& ctx)
		{
			bool res = renderer::init(ctx);
			if (!ref_prog().is_created()) {
				if (!ref_prog().build_program(ctx, "rectangle.glpr", true)) {
					std::cerr << "ERROR in rectangle_renderer::init() ... could not build program plane.glpr" << std::endl;
					return false;
				}
			}
			ref_prog().set_attribute(ctx, "depth_offset", 0.0f);
			return res;
		}
		/// set the flag, whether the position is interpreted as the rectangle center, true by default
		void rectangle_renderer::set_position_is_center(bool _position_is_center)
		{
			position_is_center = _position_is_center;
		}

		bool rectangle_renderer::validate_attributes(const context& ctx) const
		{
			const surface_render_style& srs = get_style<surface_render_style>();
			bool res = surface_renderer::validate_attributes(ctx);
			if(!has_extents) {
				ctx.error("rectangle_renderer::enable() extent attribute not set");
				res = false;
			}
			return res;
		}
		bool rectangle_renderer::enable(cgv::render::context& ctx)
		{
			if(!surface_renderer::enable(ctx))
				return false;
			const rectangle_render_style& rrs = get_style<rectangle_render_style>();
			if (!has_depth_offsets)
				ref_prog().set_attribute(ctx, "depth_offset", rrs.default_depth_offset);
			ref_prog().set_uniform(ctx, "has_rotations", has_rotations);
			ref_prog().set_uniform(ctx, "has_translations", has_translations);
			ref_prog().set_uniform(ctx, "position_is_center", position_is_center);
			ref_prog().set_uniform(ctx, "use_texture", has_texcoords);
			ref_prog().set_uniform(ctx, "viewport_height", (float)ctx.get_height());
			ref_prog().set_uniform(ctx, "texture_mode", rrs.texture_mode);
			ref_prog().set_uniform(ctx, "pixel_blend", rrs.pixel_blend);
			ref_prog().set_uniform(ctx, "border_width_in_pixel", rrs.border_width_in_pixel);
			ref_prog().set_uniform(ctx, "percentual_border_width", rrs.percentual_border_width);
			ref_prog().set_uniform(ctx, "border_mode", rrs.border_mode);
			ref_prog().set_uniform(ctx, "border_color", rrs.border_color);
			return true;
		}

		bool rectangle_renderer::disable(cgv::render::context& ctx)
		{
			if(!attributes_persist()) {
				has_extents = false;
				has_rotations = false;
				has_translations = false;
				has_texcoords = false;
				has_depth_offsets = false;
				position_is_center = true;
			}

			return surface_renderer::disable(ctx);
		}

		void rectangle_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}
		bool rectangle_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<surface_render_style*>(this)) &&
				rh.reflect_member("pixel_blend", pixel_blend) &&
				rh.reflect_member("percentual_border_width", percentual_border_width) &&
				rh.reflect_member("border_width_in_pixel", border_width_in_pixel) &&
				rh.reflect_member("default_depth_offset", default_depth_offset) &&
				rh.reflect_member("border_color", border_color);
		}
		cgv::reflect::extern_reflection_traits<rectangle_render_style, rectangle_render_style_reflect> get_reflection_traits(const cgv::render::rectangle_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<rectangle_render_style, rectangle_render_style_reflect>();
		}
	}
}



#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct rectangle_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::rectangle_render_style>::get_name())
					return false;
				cgv::render::rectangle_render_style* prs_ptr = reinterpret_cast<cgv::render::rectangle_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "texture_mode", (cgv::type::DummyEnum&)prs_ptr->texture_mode, "dropdown", "enums='replace,replace alpha,multiply color,multiply border color,colmix col+bd col,colmix bd col+col,redmix col+bd col,redmix bd col+col'");
				p->add_member_control(b, "pixel_blend", prs_ptr->pixel_blend, "value_slider", "min=0.0;max=2;ticks=true");
				p->add_member_control(b, "border_mode", (cgv::type::DummyEnum&)prs_ptr->border_mode, "dropdown", "enums='separate=0,width as ref,height as ref,min(width height)'");
				p->add_member_control(b, "border_width_in_pixel", prs_ptr->border_width_in_pixel, "value_slider", "min=-10;max=10;ticks=true");
				p->add_member_control(b, "percentual_border_width", prs_ptr->percentual_border_width, "value_slider", "min=-0.5;max=0.5;ticks=true");
				p->add_member_control(b, "border_color", prs_ptr->border_color);
				p->add_member_control(b, "default_depth_offset", prs_ptr->default_depth_offset, "value_slider", "min=0.000001;max=0.1;step=0.0000001;log=true;ticks=true");
				
				if (p->begin_tree_node("surface", prs_ptr->use_group_color, false, "level=3")) {
					p->align("\a");
					p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(prs_ptr));
					p->align("\b");
					p->end_tree_node(prs_ptr->use_group_color);
				}
				return true;
			}
		};

#include "gl/lib_begin.h"

		CGV_API cgv::gui::gui_creator_registration<rectangle_render_style_gui_creator> rectangle_rs_gc_reg("rectangle_render_style_gui_creator");

	}
}