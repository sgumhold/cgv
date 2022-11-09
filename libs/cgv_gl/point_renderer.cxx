#include "point_renderer.h"
#include <cgv_reflect_types/media/color.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		point_renderer& ref_point_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static point_renderer r;
			r.manage_singleton(ctx, "point_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* point_renderer::create_render_style() const
		{
			return new point_render_style();
		}

		point_render_style::point_render_style() : halo_color(1, 1, 1, 1)
		{
			point_size = 1.0f;
			use_group_point_size = false;
			measure_point_size_in_pixel = true;
			screen_aligned = true;

			blend_points = true;
			blend_width_in_pixel = 1.0f;
			halo_width_in_pixel = 0.0f;
			percentual_halo_width = 0.0f;
			halo_color_strength = 0.5f;
			default_depth_offset = 0.0f;
		}

		point_renderer::point_renderer()
		{
			has_point_sizes = false;
			has_group_point_sizes = false;
			has_indexed_colors = false;
			has_depth_offsets = false;
			///
			reference_point_size = 0.01f;
			y_view_angle = 45;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void point_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "point_size"))
				has_point_sizes = true;
			if (has_attribute(ctx, "color_index"))
				has_indexed_colors = true;
			if (has_attribute(ctx, "depth_offset"))
				has_depth_offsets = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void point_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::disable_attribute_array_manager(ctx, aam);
			has_point_sizes = false;
			has_indexed_colors = false;
			has_depth_offsets = false;
		}
		///
		void point_renderer::set_reference_point_size(float _reference_point_size)
		{
			reference_point_size = _reference_point_size;
		}
		///
		void point_renderer::set_y_view_angle(float _y_view_angle)
		{
			y_view_angle = _y_view_angle;
		}
		bool point_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "point.glpr", true, defines);
		}
		bool point_renderer::validate_attributes(const context& ctx) const
		{
			const point_render_style& prs = get_style<point_render_style>();
			bool res;
			if (!prs.use_group_color) {
				if (has_indexed_colors) {
					if (has_colors)
						ctx.error("point_renderer::validate_attributes() both point color and color index attributes set, using color index");
					bool tmp = has_colors;
					has_colors = true;
					res = group_renderer::validate_attributes(ctx);
					has_colors = tmp;
				}
				else
					res = group_renderer::validate_attributes(ctx);
			}
			else
				res = group_renderer::validate_attributes(ctx);
			if (!has_group_point_sizes && prs.use_group_point_size) {
				ctx.error("point_renderer::validate_attributes() group_point_sizes not set");
				res = false;
			}
			return res;
		}
		bool point_renderer::enable(cgv::render::context& ctx)
		{
			const point_render_style& prs = get_style<point_render_style>();

			bool res;
			if (!prs.use_group_color && has_indexed_colors) {
				bool tmp = has_colors;
				has_colors = true;
				res = group_renderer::enable(ctx);
				has_colors = tmp;
			}
			else
				res = group_renderer::enable(ctx);

			glPointSize(prs.point_size);
			if (prs.blend_points) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			if (ref_prog().is_linked()) {
				if (!has_point_sizes)
					ref_prog().set_attribute(ctx, "point_size", prs.point_size);
				if (!has_depth_offsets)
					ref_prog().set_attribute(ctx, "depth_offset", prs.default_depth_offset);
				ref_prog().set_uniform(ctx, "use_color_index", has_indexed_colors);
				ref_prog().set_uniform(ctx, "measure_point_size_in_pixel", prs.measure_point_size_in_pixel);
				ref_prog().set_uniform(ctx, "screen_aligned", prs.screen_aligned);
				ref_prog().set_uniform(ctx, "reference_point_size", reference_point_size);
				ref_prog().set_uniform(ctx, "use_group_point_size", prs.use_group_point_size);
				ref_prog().set_uniform(ctx, "viewport_height", (float)ctx.get_height());
				ref_prog().set_uniform(ctx, "blend_width_in_pixel", prs.blend_width_in_pixel);
				ref_prog().set_uniform(ctx, "percentual_halo_width", 0.01f*prs.percentual_halo_width);
				ref_prog().set_uniform(ctx, "halo_width_in_pixel", prs.halo_width_in_pixel);
				ref_prog().set_uniform(ctx, "halo_color", prs.halo_color);
				ref_prog().set_uniform(ctx, "halo_color_strength", prs.halo_color_strength);
			}
			return res;
		}

		bool point_renderer::disable(cgv::render::context& ctx)
		{
			const point_render_style& prs = get_style<point_render_style>();
			if (prs.blend_points) {
				glDisable(GL_BLEND);
			}
			if (!attributes_persist()) {
				has_indexed_colors = false;
				has_point_sizes = false;
				has_depth_offsets = false;
			}
			return group_renderer::disable(ctx);
		}

		void point_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}

		bool point_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<group_render_style*>(this)) &&
				rh.reflect_member("point_size", point_size) &&
				rh.reflect_member("use_group_point_size", use_group_point_size) &&
				rh.reflect_member("measure_point_size_in_pixel", measure_point_size_in_pixel) &&
				rh.reflect_member("screen_aligned", screen_aligned) &&
				rh.reflect_member("default_depth_offset", default_depth_offset) &&
				rh.reflect_member("blend_points", blend_points) &&
				rh.reflect_member("blend_width_in_pixel", blend_width_in_pixel) &&
				rh.reflect_member("halo_width_in_pixel", halo_width_in_pixel) &&
				rh.reflect_member("halo_color", halo_color) &&
				rh.reflect_member("halo_color_strength", halo_color_strength) &&
				rh.reflect_member("percentual_halo_width", percentual_halo_width);
		}
		cgv::reflect::extern_reflection_traits<point_render_style, point_render_style_reflect> get_reflection_traits(const cgv::render::point_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<point_render_style, point_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct point_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::point_render_style>::get_name())
					return false;
				cgv::render::point_render_style* prs_ptr = reinterpret_cast<cgv::render::point_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				p->add_member_control(b, "Screen Aligned", prs_ptr->screen_aligned, "toggle");
				p->add_member_control(b, "Point Size", prs_ptr->point_size, "value_slider", "label='';w=130;min=0.01;max=50;log=true;ticks=true", "");
				p->add_member_control(b, "px", prs_ptr->measure_point_size_in_pixel, "toggle", "w=16", "");
				p->add_member_control(b, "Blend", prs_ptr->blend_points, "toggle", "w=50");
				p->add_member_control(b, "Default Depth Offset", prs_ptr->default_depth_offset, "value_slider", "min=0.000001;max=0.1;step=0.0000001;log=true;ticks=true");
				bool show = p->begin_tree_node("Halo", prs_ptr->halo_color, false, "options='w=120';level=3;align=''");
				p->add_member_control(b, "Color", prs_ptr->halo_color, "", "w=50");
				if (show) {
					p->align("\a");
					p->add_member_control(b, "Halo Color Strength", prs_ptr->halo_color_strength, "value_slider", "min=0;max=1;ticks=true");
					p->add_member_control(b, "Halo Width in Pixel", prs_ptr->halo_width_in_pixel, "value_slider", "min=-10;max=10;ticks=true");
					p->add_member_control(b, "Percentual Halo Width", prs_ptr->percentual_halo_width, "value_slider", "min=-100;max=100;ticks=true");
					p->add_member_control(b, "Blend Width in Pixel", prs_ptr->blend_width_in_pixel, "value_slider", "min=0;max=3;ticks=true");
					p->align("\b");
					p->end_tree_node(prs_ptr->halo_color);
				}
				if (p->begin_tree_node("Use of Group Information", prs_ptr->use_group_color, false, "level=3")) {
					p->align("\a");
					p->add_gui("group_render_style", *static_cast<cgv::render::group_render_style*>(prs_ptr));
					p->align("\b");
					p->end_tree_node(prs_ptr->use_group_color);
				}
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<point_render_style_gui_creator> point_rs_gc_reg("point_render_style_gui_creator");

	}
}