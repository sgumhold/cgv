#include "line_renderer.h"
#include <cgv_reflect_types/media/color.h>
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		
		line_renderer& ref_line_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static line_renderer r;
			r.manage_singleton(ctx, "line_renderer", ref_count, ref_count_change);
			return r;
		}
		render_style* line_renderer::create_render_style() const
		{
			return new line_render_style();
		}

		line_render_style::line_render_style() : default_color(0, 1, 1, 1)
		{
			default_normal = vec3(0.0f,0.0f,1.0f);
			default_color = rgba(1.0f);
			default_depth_offset = 0.0f;
			default_line_width = 1.0f;
			blend_lines = false;
			halo_color = rgba(0.0f,0.0f,0.0f,1.0);
			halo_width_in_pixel = 0.0f;
			percentual_halo_width = 0.0f;
			screen_aligned = true;
			measure_line_width_in_pixel = true;
			reference_line_width = 0.001f;
			blend_width_in_pixel = 0.0f;
			halo_color_strength = 1.0f;
		}
		/// call this before setting attribute arrays to manage attribute array in given manager
		void line_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "normal"))
				has_normals = true;
			if (has_attribute(ctx, "line_width"))
				has_line_widths = true;
			if (has_attribute(ctx, "depth_offset"))
				has_depth_offsets = true;
		}
		/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
		void line_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			group_renderer::disable_attribute_array_manager(ctx, aam);
			has_normals = false;
			has_line_widths = false;
			has_depth_offsets = false;
		}

		line_renderer::line_renderer()
		{
			has_normals = false;
			has_line_widths = false;
			has_depth_offsets = false;
		}
		bool line_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "line.glpr", true, defines);
		}

		bool line_renderer::init(context& ctx)
		{
			bool res = renderer::init(ctx);
			int li;
			li = get_prog_attribute_location(ctx, "normal", false);
			if (li != -1)
				ref_prog().set_attribute(ctx, li, vec3(0.0f,0.0f,1.0f));
			li = get_prog_attribute_location(ctx, "depth_offset", false);
			if (li != -1)
				ref_prog().set_attribute(ctx, li, 0.0f);
			li = get_prog_attribute_location(ctx, "line_width", false);
			if (li != -1)
				ref_prog().set_attribute(ctx, li, 1.0f);
			return res;
		}

		bool line_renderer::enable(context& ctx)
		{
			const line_render_style& lrs = get_style<line_render_style>();
			if (!group_renderer::enable(ctx))
				return false;
			// set program attributes
			if (!has_colors)
				ctx.set_color(lrs.default_color);
			if (!has_normals) {
				int li = get_prog_attribute_location(ctx, "normal", false);
				if (li != -1)
					ref_prog().set_attribute(ctx, li, lrs.default_normal);
			}
			if (!has_depth_offsets) {
				int li = get_prog_attribute_location(ctx, "depth_offset", false);
				if (li != -1)
					ref_prog().set_attribute(ctx, li, lrs.default_depth_offset);
			}
			if (!has_line_widths) {
				int li = get_prog_attribute_location(ctx, "line_width", false);
				if (li != -1)
					ref_prog().set_attribute(ctx, li, lrs.default_line_width);
			}
			// configure opengl
			if (lrs.blend_lines) {
				lrs.is_blend = glIsEnabled(GL_BLEND);
				glGetIntegerv(GL_BLEND_DST, reinterpret_cast<GLint*>(&lrs.blend_dst));
				glGetIntegerv(GL_BLEND_SRC, reinterpret_cast<GLint*>(&lrs.blend_src));
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			// set program uniforms
			ref_prog().set_uniform(ctx, "halo_color", lrs.halo_color);
			ref_prog().set_uniform(ctx, "halo_width_in_pixel", lrs.halo_width_in_pixel);
			ref_prog().set_uniform(ctx, "percentual_halo_width", 0.01f * lrs.percentual_halo_width);
			ref_prog().set_uniform(ctx, "screen_aligned", lrs.screen_aligned);
			ref_prog().set_uniform(ctx, "measure_line_width_in_pixel", lrs.measure_line_width_in_pixel);
			ref_prog().set_uniform(ctx, "reference_line_width", lrs.reference_line_width);
			ref_prog().set_uniform(ctx, "blend_width_in_pixel", lrs.blend_width_in_pixel);
			ref_prog().set_uniform(ctx, "viewport_height", (float)ctx.get_height());
			ref_prog().set_uniform(ctx, "halo_color_strength", lrs.halo_color_strength);
			return true;
		}

		bool line_renderer::disable(context& ctx)
		{
			if (!attributes_persist()) {
				has_normals = false;
				has_line_widths = false;
				has_depth_offsets = false;
			}
			const line_render_style& lrs = get_style<line_render_style>();
			if (lrs.blend_lines) {
				if (!lrs.is_blend)
					glDisable(GL_BLEND);
				glBlendFunc(lrs.blend_src, lrs.blend_dst);
			}
			return group_renderer::disable(ctx);
		}

		void line_renderer::draw(context& ctx, size_t start, size_t count, bool use_strips, bool use_adjacency, uint32_t strip_restart_index)
		{	
			draw_impl(ctx, use_strips ? (use_adjacency ? PT_LINE_STRIP_ADJACENCY : PT_LINE_STRIP) : (use_adjacency ? PT_LINES_ADJACENCY : PT_LINES), start, count, use_strips, use_adjacency, strip_restart_index);
		}

		bool line_render_style_reflect::self_reflect(cgv::reflect::reflection_handler& rh)
		{
			return
				rh.reflect_base(*static_cast<cgv::render::group_render_style*>(this)) &&
				rh.reflect_member("default_normal", default_normal) &&
				rh.reflect_member("default_color", default_color) &&
				rh.reflect_member("default_line_width", default_line_width) &&
				rh.reflect_member("default_depth_offset", default_depth_offset) &&
				rh.reflect_member("blend_lines", blend_lines) &&
				rh.reflect_member("halo_color", halo_color) &&
				rh.reflect_member("halo_width_in_pixel", halo_width_in_pixel) &&
				rh.reflect_member("percentual_halo_width", percentual_halo_width) &&
				rh.reflect_member("screen_aligned", screen_aligned) &&
				rh.reflect_member("measure_line_width_in_pixel", measure_line_width_in_pixel) &&
				rh.reflect_member("reference_line_width", reference_line_width) &&
				rh.reflect_member("blend_width_in_pixel", blend_width_in_pixel) &&
				rh.reflect_member("halo_color_strength", halo_color_strength);
		}

		cgv::reflect::extern_reflection_traits<line_render_style, line_render_style_reflect> get_reflection_traits(const line_render_style&)
		{
			return cgv::reflect::extern_reflection_traits<line_render_style, line_render_style_reflect>();
		}
	}
}

#include <cgv/gui/provider.h>

namespace cgv {
	namespace gui {

		struct line_render_style_gui_creator : public gui_creator
		{
			/// attempt to create a gui and return whether this was successful
			bool create(provider* p, const std::string& label,
				void* value_ptr, const std::string& value_type,
				const std::string& gui_type, const std::string& options, bool*)
			{
				if (value_type != cgv::type::info::type_name<cgv::render::line_render_style>::get_name())
					return false;
				cgv::render::line_render_style* lrs_ptr = reinterpret_cast<cgv::render::line_render_style*>(value_ptr);
				cgv::base::base* b = dynamic_cast<cgv::base::base*>(p);
				p->add_gui("Default Normal", lrs_ptr->default_normal, "direction", "options='min=-1;max=1;ticks=true'");
				p->add_member_control(b, "Default Color", lrs_ptr->default_color);
				p->add_member_control(b, "Screen Aligned", lrs_ptr->screen_aligned, "toggle");
				p->add_member_control(b, "Default Line Width", lrs_ptr->default_line_width, "value_slider", "min=1;max=20;log=true;ticks=true;w=130", "");
				p->add_member_control(b, "px", lrs_ptr->measure_line_width_in_pixel, "toggle", "w=16", "");
				p->add_member_control(b, "Blend", lrs_ptr->blend_lines, "toggle", "w=50");				
				p->add_member_control(b, "Reference Line Width", lrs_ptr->reference_line_width, "value_slider", "min=0.0000001;max=1;step=0.00000001;log=true;ticks=true");
				p->add_member_control(b, "Default Depth Offset", lrs_ptr->default_depth_offset, "value_slider", "min=-0.001;max=0.001;step=0.00000001;log=true;ticks=true");
				bool show = p->begin_tree_node("Halo", lrs_ptr->halo_color, false, "options='w=120';level=3;align=''");
				p->add_member_control(b, "Color", lrs_ptr->halo_color, "", "w=50");
				if (show) {
					p->align("\a");
					p->add_member_control(b, "Halo Color Strength", lrs_ptr->halo_color_strength, "value_slider", "min=0;max=1;ticks=true");
					p->add_member_control(b, "Halo Width in Pixel", lrs_ptr->halo_width_in_pixel, "value_slider", "min=-10;max=10;ticks=true");
					p->add_member_control(b, "Percentual Halo Width", lrs_ptr->percentual_halo_width, "value_slider", "min=-100;max=100;ticks=true");
					p->add_member_control(b, "Blend Width in Pixel", lrs_ptr->blend_width_in_pixel, "value_slider", "min=0;max=3;ticks=true");
					p->align("\b");
					p->end_tree_node(lrs_ptr->halo_color);
				}
				if (p->begin_tree_node("Use of Group Information", lrs_ptr->use_group_color, false, "level=3")) {
					p->align("\a");
					p->add_gui("group render style", *static_cast<cgv::render::group_render_style*>(lrs_ptr));
					p->align("\b");
					p->end_tree_node(lrs_ptr->use_group_color);
				}
				return true;
			}
		};

#include "gl/lib_begin.h"

CGV_API cgv::gui::gui_creator_registration<line_render_style_gui_creator> line_rs_gc_reg("line_render_style_gui_creator");

	}
}