#include "surfel_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		surfel_renderer& ref_surfel_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static surfel_renderer r;
			r.manage_singleton(ctx, "surfel_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* surfel_renderer::create_render_style() const
		{
			return new surfel_render_style();
		}

		surfel_render_style::surfel_render_style() : halo_color(1, 1, 1, 1)
		{
			surface_offset = 0.0f;
			point_size = 1.0f;
			use_group_point_size = false;
			measure_point_size_in_pixel = true;
			blend_width_in_pixel = 1.0f;
			halo_width_in_pixel = 0.0f;
			percentual_halo_width = 0.0;
			halo_color_strength = 0.5f;
			blend_points = true;
			orient_splats = true;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void surfel_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "point_size"))
				has_point_sizes = true;
			if (has_attribute(ctx, "color_index"))
				has_indexed_colors = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void surfel_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::disable_attribute_array_manager(ctx, aam);
			has_point_sizes = false;
			has_indexed_colors = false;
		}

		surfel_renderer::surfel_renderer()
		{
			has_point_sizes = false;
			has_group_point_sizes = false;
			has_indexed_colors = false;
			///
			reference_point_size = 0.01f;
			y_view_angle = 45;
		}
		void surfel_renderer::set_reference_point_size(float _reference_point_size)
		{
			reference_point_size = _reference_point_size;
		}
		void surfel_renderer::set_y_view_angle(float _y_view_angle)
		{
			y_view_angle = _y_view_angle;
		}
		bool surfel_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "surfel.glpr", true, defines);
		}
		bool surfel_renderer::validate_attributes(const context& ctx) const
		{
			const surfel_render_style& srs = get_style<surfel_render_style>();
			bool res;
			if (!srs.use_group_color) {
				if (has_indexed_colors) {
					if (has_colors)
						ctx.error("surfel_renderer::validate_attributes() both point color and color index attributes set, using color index");
					bool tmp = has_colors;
					has_colors = true;
					res = surface_renderer::validate_attributes(ctx);
					has_colors = tmp;
				}
				else
					res = surface_renderer::validate_attributes(ctx);
			}
			else
				res = surface_renderer::validate_attributes(ctx);
			if (!has_group_point_sizes && srs.use_group_point_size) {
				ctx.error("surfel_renderer::validate_attributes() group_point_sizes not set");
				res = false;
			}
			return res;
		}
		bool surfel_renderer::enable(cgv::render::context& ctx)
		{
			const surfel_render_style& srs = get_style<surfel_render_style>();

			bool res;
			if (!srs.use_group_color && has_indexed_colors) {
				bool tmp = has_colors;
				has_colors = true;
				res = surface_renderer::enable(ctx);
				has_colors = tmp;
			}
			else
				res = surface_renderer::enable(ctx);

			glPointSize(srs.point_size);
			if (srs.blend_points) {
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			if (ref_prog().is_linked()) {
				if (!has_point_sizes)
					ref_prog().set_attribute(ctx, "point_size", srs.point_size);
				ref_prog().set_uniform(ctx, "use_color_index", has_indexed_colors);
				ref_prog().set_uniform(ctx, "measure_point_size_in_pixel", srs.measure_point_size_in_pixel);
				ref_prog().set_uniform(ctx, "reference_point_size", reference_point_size);
				ref_prog().set_uniform(ctx, "use_group_point_size", srs.use_group_point_size);
				ref_prog().set_uniform(ctx, "orient_splats", has_normals ? srs.orient_splats : false);
				float pixel_extent_per_depth = (float)(2.0*tan(0.5*0.0174532925199*y_view_angle) / ctx.get_height());
				ref_prog().set_uniform(ctx, "pixel_extent_per_depth", pixel_extent_per_depth);
				ref_prog().set_uniform(ctx, "blend_width_in_pixel", srs.blend_width_in_pixel);
				ref_prog().set_uniform(ctx, "percentual_halo_width", 0.01f*srs.percentual_halo_width);
				ref_prog().set_uniform(ctx, "halo_width_in_pixel", srs.halo_width_in_pixel);
				ref_prog().set_uniform(ctx, "halo_color", srs.halo_color);
				ref_prog().set_uniform(ctx, "halo_color_strength", srs.halo_color_strength);
				ref_prog().set_uniform(ctx, "surface_offset", srs.surface_offset);
			}
			return res;
		}

		bool surfel_renderer::disable(cgv::render::context& ctx)
		{
			const surfel_render_style& srs = get_style<surfel_render_style>();
			if (srs.blend_points) {
				glDisable(GL_BLEND);
			}
			if (srs.culling_mode != CM_OFF) {
				glDisable(GL_CULL_FACE);
			}
			if (!attributes_persist()) {
				has_indexed_colors = false;
				has_point_sizes = false;
			}
			return surface_renderer::disable(ctx);
		}

		void surfel_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{
			draw_impl(ctx, PT_POINTS, start, count, false, false, -1);
		}

		bool surfel_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<cgv::render::surface_render_style*>(this)) &&
				rh.reflect_member("point_size", point_size) &&
				rh.reflect_member("use_group_point_size", use_group_point_size) &&
				rh.reflect_member("measure_point_size_in_pixel", measure_point_size_in_pixel) &&
				rh.reflect_member("blend_points", blend_points) &&
				rh.reflect_member("orient_splats", orient_splats) &&
				rh.reflect_member("blend_width_in_pixel", blend_width_in_pixel) &&
				rh.reflect_member("halo_width_in_pixel", halo_width_in_pixel) &&
				rh.reflect_member("halo_color", halo_color) &&
				rh.reflect_member("halo_color_strength", halo_color_strength) &&
				rh.reflect_member("percentual_halo_width", percentual_halo_width);
		}

		cgv::reflect::extern_reflection_traits<surfel_render_style, surfel_render_style_reflect> get_reflection_traits(const surfel_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<surfel_render_style, surfel_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct surfel_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::surfel_render_style>::get_name())
					return false;
				cgv::render::surfel_render_style* srs_ptr = reinterpret_cast<cgv::render::surfel_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);

				bool show = p->begin_tree_node("Splatting", srs_ptr->point_size, false, "options='w=60';level=3;align=''");
				p->add_member_control(b, "Point Size", srs_ptr->point_size, "value_slider", "label='';w=130;min=0.01;max=50;log=true;ticks=true", "");
				p->add_member_control(b, "px", srs_ptr->measure_point_size_in_pixel, "toggle", "w=16");
				if (show) {
					p->align("\a");
					p->add_member_control(b, "Blend", srs_ptr->blend_points, "toggle", "w=90", " ");
					p->add_member_control(b, "Orient", srs_ptr->orient_splats, "toggle", "w=90");
					p->add_member_control(b, "Surface Offset", srs_ptr->surface_offset, "value_slider", "min=0;max=0.1;step=0.0001;log=true;ticks=true");
					p->align("\b");
					p->end_tree_node(srs_ptr->point_size);
				}
				show = p->begin_tree_node("Halo", srs_ptr->halo_color, false, "options='w=120';level=3;align=''");
				p->add_member_control(b, "Color", srs_ptr->halo_color, "", "w=50");
				if (show) {
					p->align("\a");
					p->add_member_control(b, "Halo Color Strength", srs_ptr->halo_color_strength, "value_slider", "min=0;max=1;ticks=true");
					p->add_member_control(b, "Halo Width in Pixel", srs_ptr->halo_width_in_pixel, "value_slider", "min=-10;max=10;ticks=true");
					p->add_member_control(b, "Percentual Halo Width", srs_ptr->percentual_halo_width, "value_slider", "min=-100;max=100;ticks=true");
					p->add_member_control(b, "Blend Width in Pixel", srs_ptr->blend_width_in_pixel, "value_slider", "min=0;max=3;ticks=true");
					p->align("\b");
					p->end_tree_node(srs_ptr->halo_color);
				}
				if (p->begin_tree_node("Surface", srs_ptr->use_group_color, false, "level=3")) {
					p->align("\a");
					p->add_gui("surface_render_style", *static_cast<cgv::render::surface_render_style*>(srs_ptr));
					p->align("\b");
					p->end_tree_node(srs_ptr->use_group_color);
				}
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<surfel_render_style_gui_creator> surfel_rs_gc_reg("surfel_render_style_gui_creator");

	}
}