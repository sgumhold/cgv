#include "arrow_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		arrow_renderer& ref_arrow_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static arrow_renderer r;
			r.manage_singleton(ctx, "arrow_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* arrow_renderer::create_render_style() const
		{
			return new arrow_render_style();
		}

		arrow_render_style::arrow_render_style() 
		{
			radius_lower_bound = 0.00001f;
			radius_relative_to_length = 0.1f;
			head_radius_scale = 2.0f;
			head_length_mode = AHLM_MINIMUM_OF_RADIUS_AND_LENGTH;
			head_length_relative_to_radius = 2.0f;
			head_length_relative_to_length = 0.3f;
			length_scale = 1.0f;
			color_scale = 1.0f;
			normalize_length = false;
			relative_location_of_position = 0.0f;
			length_eps = 0.000001f;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void arrow_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "direction"))
				has_directions = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void arrow_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::disable_attribute_array_manager(ctx, aam);
			has_directions = false;
		}
		arrow_renderer::arrow_renderer()
		{
			has_directions = false;
			position_is_center = false;
			direction_is_end_point = false;
		}
		bool arrow_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "arrow.glpr", true, defines);
		}

		bool arrow_renderer::validate_attributes(const context& ctx) const
		{
			const arrow_render_style& ars = get_style<arrow_render_style>();
			if (!surface_renderer::validate_attributes(ctx))
				return false;

			if (!has_directions) {
				ctx.error("arrow_renderer::validate_attributes() neither direction nor end_point attribute set");
				return false;
			}
			return true;
		}
		bool arrow_renderer::enable(cgv::render::context& ctx)
		{
			const arrow_render_style& ars = get_style<arrow_render_style>();

			bool res = surface_renderer::enable(ctx);

			if (ref_prog().is_linked()) {
				ref_prog().set_uniform(ctx, "radius_lower_bound", ars.radius_lower_bound);
				ref_prog().set_uniform(ctx, "radius_relative_to_length", ars.radius_relative_to_length);
				ref_prog().set_uniform(ctx, "head_radius_scale", ars.head_radius_scale);
				ref_prog().set_uniform(ctx, "head_length_mode", (int&)ars.head_length_mode);
				ref_prog().set_uniform(ctx, "head_length_relative_to_radius", ars.head_length_relative_to_radius);
				ref_prog().set_uniform(ctx, "head_length_relative_to_length", ars.head_length_relative_to_length);
				ref_prog().set_uniform(ctx, "length_scale", ars.length_scale);
				ref_prog().set_uniform(ctx, "color_scale", ars.color_scale);
				ref_prog().set_uniform(ctx, "length_eps", ars.length_eps);
				ref_prog().set_uniform(ctx, "normalize_length", ars.normalize_length);
				ref_prog().set_uniform(ctx, "direction_is_end_point", direction_is_end_point);
				ref_prog().set_uniform(ctx, "relative_location_of_position", ars.relative_location_of_position);
			}
			return res;
		}

		bool arrow_renderer::disable(cgv::render::context& ctx)
		{
			const arrow_render_style& ars = get_style<arrow_render_style>();
			if (!attributes_persist()) {
				has_directions = false;
			}
			return surface_renderer::disable(ctx);
		}

		void arrow_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			const arrow_render_style& ars = get_style<arrow_render_style>();
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}

		bool arrow_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<cgv::render::surface_render_style*>(this)) &&
				rh.reflect_member("radius_lower_bound", radius_lower_bound) &&
				rh.reflect_member("radius_relative_to_length", radius_relative_to_length) &&
				rh.reflect_member("head_radius_scale", head_radius_scale) &&
				rh.reflect_member("head_length_relative_to_radius", head_length_relative_to_radius) &&
				rh.reflect_member("head_length_relative_to_length", head_length_relative_to_length) &&
				rh.reflect_member("length_scale", length_scale) &&
				rh.reflect_member("color_scale", color_scale) &&
				rh.reflect_member("length_eps", length_eps) &&
				rh.reflect_member("normalize_length", normalize_length) &&
				rh.reflect_member("relative_location_of_position", relative_location_of_position);
		}

		cgv::reflect::extern_reflection_traits<arrow_render_style, arrow_render_style_reflect> get_reflection_traits(const arrow_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<arrow_render_style, arrow_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct arrow_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::arrow_render_style>::get_name())
					return false;
				cgv::render::arrow_render_style* ars_ptr = reinterpret_cast<cgv::render::arrow_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "Relative Location of Position", ars_ptr->relative_location_of_position, "value_slider", "min=0;max=1;ticks=true");
				p->add_member_control(b, "Color Scale", ars_ptr->color_scale, "value_slider", "min=0.01;max=100;log=true;ticks=true");
				if (p->begin_tree_node("Length", ars_ptr->length_scale, true, "level=3")) {
					p->align("\a");
					p->add_member_control(b, "Normalize Length", ars_ptr->normalize_length, "toggle");
					p->add_member_control(b, "Length Scale", ars_ptr->length_scale, "value_slider", "min=0.01;max=100;log=true;ticks=true");
					p->add_member_control(b, "Length Epsilon", ars_ptr->length_eps, "value_slider", "min=0.00000001;step=0.000000001;max=1;log=true;ticks=true");
					p->align("\b");
					p->end_tree_node(ars_ptr->length_scale);
				}
				if (p->begin_tree_node("Radius", ars_ptr->radius_lower_bound, true, "level=3")) {
					p->align("\a");
					p->add_member_control(b, "Radius Relative to Length", ars_ptr->radius_relative_to_length, "value_slider", "min=0;max=1;ticks=true");
					p->add_member_control(b, "Radius Lower Bound", ars_ptr->radius_lower_bound, "value_slider", "min=0.00000001;step=0.000000001;max=0.01;log=true;ticks=true");
					p->align("\b");
					p->end_tree_node(ars_ptr->radius_lower_bound);
				}
				if (p->begin_tree_node("Head Radius", ars_ptr->head_radius_scale, true, "level=3")) {
					p->align("\a");
					p->add_member_control(b, "Head Length Mode", ars_ptr->head_length_mode, "dropdown", "enums='Relative to Radius=1,Relative to Length=2,Minimum of Radius and Length=3'");
					p->add_member_control(b, "Head Length Relative to Radius", ars_ptr->head_length_relative_to_radius, "value_slider", "min=0.1;max=5;ticks=true");
					p->add_member_control(b, "Head Length Relative to Length", ars_ptr->head_length_relative_to_length, "value_slider", "min=0;max=1;ticks=true");
					p->add_member_control(b, "Head Radius Scale", ars_ptr->head_radius_scale, "value_slider", "min=1;max=3;ticks=true");
					p->align("\b");
					p->end_tree_node(ars_ptr->head_radius_scale);
				}
				if (p->begin_tree_node("Surface Rendering", ars_ptr->use_group_color, false, "level=3")) {
					p->align("\a");
					p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(ars_ptr));
					p->align("\b");
					p->end_tree_node(ars_ptr->use_group_color);
				}
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<arrow_render_style_gui_creator> arrow_rs_gc_reg("arrow_render_style_gui_creator");

	}
}