#include "box_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		/// default constructor sets default extent to (1,1,1)
		box_render_style::box_render_style()
		{
			default_extent = vec3(1.0f);
			relative_anchor = vec3(0.0f);
		}

		box_renderer& ref_box_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static box_renderer r;
			r.manage_singleton(ctx, "box_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* box_renderer::create_render_style() const
		{
			return new surface_render_style();
		}

		box_renderer::box_renderer()
		{
			has_extents = false;
			has_radii = false;
			position_is_center = true;
			has_translations = false;
			has_rotations = false;
			has_secondary_colors = false;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void box_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "extent"))
				has_extents = true;
			if (has_attribute(ctx, "secondary_color"))
				has_secondary_colors = true;
			if (has_attribute(ctx, "radius"))
				has_radii = true;
			if (has_attribute(ctx, "translation"))
				has_translations = true;
			if (has_attribute(ctx, "rotation"))
				has_rotations = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void box_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::disable_attribute_array_manager(ctx, aam);
			has_extents = false;
			has_radii = false;
			has_translations = false;
			has_rotations = false;
			has_secondary_colors = false;
		}
		/// set the flag, whether the position is interpreted as the box center
		void box_renderer::set_position_is_center(bool _position_is_center)
		{
			position_is_center = _position_is_center;
		}
		/// build box program
		bool box_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "box.glpr", true, defines);
		}
		/// 
		bool box_renderer::enable(context& ctx)
		{
			if (!surface_renderer::enable(ctx))
				return false;
			ref_prog().set_uniform(ctx, "position_is_center", position_is_center);
			ref_prog().set_uniform(ctx, "has_rotations", has_rotations);
			ref_prog().set_uniform(ctx, "has_translations", has_translations);
			ref_prog().set_uniform(ctx, "has_radii", has_radii);
			ref_prog().set_uniform(ctx, "has_secondary_colors", has_secondary_colors);
			const auto& brs = get_style<box_render_style>();
			if (ref_prog().get_uniform_location(ctx, "default_radius") != -1)
				ref_prog().set_uniform(ctx, "default_radius", brs.default_radius);
			if (ref_prog().get_uniform_location(ctx, "relative_anchor") != -1)
				ref_prog().set_uniform(ctx, "relative_anchor", brs.relative_anchor);
			if (!has_extents)
				ref_prog().set_attribute(ctx, "extent", brs.default_extent);
			return true;
		}
		///
		bool box_renderer::disable(context& ctx)
		{
			if (!attributes_persist()) {
				has_extents = false;
				has_radii = false;
				position_is_center = true;
				has_rotations = false;
				has_translations = false;
			}

			return surface_renderer::disable(ctx);
		}

		void box_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}

		bool box_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<surface_render_style*>(this)) &&
				rh.reflect_member("default_extent", default_extent) &&
				rh.reflect_member("default_radius", default_radius) &&
				rh.reflect_member("relative_anchor", relative_anchor);
		}

		void box_renderer::update_defines(shader_define_map& defines)
		{
			const box_render_style& brs = get_style<box_render_style>();
			shader_code::set_define(defines, "ROUNDING", brs.rounding, false);
		}
		cgv::reflect::extern_reflection_traits<box_render_style, box_render_style_reflect> get_reflection_traits(const box_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<box_render_style, box_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct box_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::box_render_style>::get_name())
					return false;
				cgv::render::box_render_style* brs_ptr = reinterpret_cast<cgv::render::box_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
				p->add_member_control(b, "Default Extent X", brs_ptr->default_extent[0], "value_slider", "min=0.001;max=1000;log=true;ticks=true");
				p->add_member_control(b, "Default Extent Y", brs_ptr->default_extent[1], "value_slider", "min=0.001;max=1000;log=true;ticks=true");
				p->add_member_control(b, "Default Extent Z", brs_ptr->default_extent[2], "value_slider", "min=0.001;max=1000;log=true;ticks=true");
				p->add_member_control(b, "Relative Anchor X", brs_ptr->relative_anchor[0], "value_slider", "min=-1;max=1;ticks=true");
				p->add_member_control(b, "Relative Anchor Y", brs_ptr->relative_anchor[1], "value_slider", "min=-1;max=1;ticks=true");
				p->add_member_control(b, "Relative Anchor Z", brs_ptr->relative_anchor[2], "value_slider", "min=-1;max=1;ticks=true");
				p->add_member_control(b, "Rounding", brs_ptr->rounding, "toggle");
				p->add_member_control(b, "Default Radius", brs_ptr->default_radius, "value_slider", "min=0.0;max=10;step=0.0001;log=true;ticks=true");
				p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(brs_ptr));
				return true;
			}
		};

#include "gl/lib_begin.h"

		CGV_API cgv::gui::gui_creator_registration<box_render_style_gui_creator> box_rs_gc_reg("box_render_style_gui_creator");
	}
}