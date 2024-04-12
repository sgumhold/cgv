#include "rectangle_renderer.h"
#include <cgv_gl/gl/gl.h>
#include <cgv_gl/gl/gl_tools.h>

namespace cgv {
	namespace render {
		/// allow to use RectangleTextureMode in self reflection
		cgv::reflect::enum_reflection_traits<RectangleTextureMode> get_reflection_traits(const RectangleTextureMode& texture_mode)
		{
			return cgv::reflect::enum_reflection_traits<RectangleTextureMode>(
				"RTM_REPLACE=0,"
				"RTM_REPLACE_ALPHA,"
				"RTM_MULTIPLY_COLOR,"
				"RTM_MULTIPLY_SECONDARY_COLOR,"
				"RTM_MULTIPLY_BORDER_COLOR,"
				"RTM_MIX_COLOR_AND_SECONDARY_COLOR,"
				"RTM_MIX_COLOR_AND_BORDER_COLOR,"
				"RTM_MIX_SECONDARY_COLOR_AND_COLOR,"
				"RTM_MIX_BORDER_COLOR_AND_COLOR,"
				"RTM_RED_MIX_COLOR_AND_SECONDARY_COLOR,"
				"RTM_RED_MIX_COLOR_AND_BORDER_COLOR,"
				"RTM_RED_MIX_SECONDARY_COLOR_AND_COLOR,"
				"RTM_RED_MIX_BORDER_COLOR_AND_COLOR"
			);
		}
		/// allow to use RectangleBoderMode in self reflection
		cgv::reflect::enum_reflection_traits<RectangleBoderMode> get_reflection_traits(const RectangleBoderMode& border_mode)
		{
			return cgv::reflect::enum_reflection_traits<RectangleBoderMode>(
				"RBM_SEPARATE=0,"
				"RBM_WIDTH,"
				"RBM_HEIGHT,"
				"RBM_MIN"
			);
		}
		rectangle_renderer& ref_rectangle_renderer(context& ctx, int ref_count_change)
		{
			static int ref_count = 0;
			static rectangle_renderer r;
			r.manage_singleton(ctx, "rectangle_renderer", ref_count, ref_count_change);
			return r;
		}

		render_style* rectangle_renderer::create_render_style() const
		{
			return new rectangle_render_style();
		}

		rectangle_render_style::rectangle_render_style()
		{
			position_is_center = true;
			default_secondary_color = rgba(0.5f, 0.5f, 0.5f, 1.0f);
			default_border_color = rgba(0.0f, 0.0f, 0.0f, 1.0f);
			border_width_in_pixel = 0.0f;
			percentual_border_width = 0.0f;
			border_mode = RBM_MIN;
			pixel_blend = 0.0f;
			texture_mode = RTM_REPLACE;
			default_depth_offset = 0.0f;
			blend_rectangles = false;
			//is_blend = GL_FALSE;
			//blend_src = blend_dst = 0;
		}
		rectangle_renderer::rectangle_renderer() {
 			has_extents = false;
			has_secondary_colors = false;
			has_border_colors = false;
			has_border_infos = false;
			has_translations = false;
			has_rotations = false;
			has_texcoords = false;
			has_depth_offsets = false;
			y_view_angle = 45;
		}

		void rectangle_renderer::set_y_view_angle(float _y_view_angle)
		{
			y_view_angle = _y_view_angle;
		}

		void rectangle_renderer::set_secondary_color_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_secondary_colors = true;
			set_attribute_array(ctx, "secondary_color", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}
		void rectangle_renderer::set_border_color_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_border_colors = true;
			set_attribute_array(ctx, "border_color", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}
		void rectangle_renderer::rectangle_renderer::set_border_info_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes)
		{
			has_border_infos = true;
			set_attribute_array(ctx, "border_info", element_type, vbo, offset_in_bytes, nr_elements, stride_in_bytes);
		}
		void rectangle_renderer::enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::enable_attribute_array_manager(ctx, aam);
			if (has_attribute(ctx, "extent"))
				has_extents = true;
			if (has_attribute(ctx, "secondary_color"))
				has_secondary_colors = true;
			if (has_attribute(ctx, "border_color"))
				has_border_colors = true;
			if (has_attribute(ctx, "border_info"))
				has_border_infos = true;
			if (has_attribute(ctx, "translation"))
				has_translations = true;
			if (has_attribute(ctx, "rotation"))
				has_rotations = true;
			if (has_attribute(ctx, "depth_offset"))
				has_depth_offsets = true;
		}

		void rectangle_renderer::disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam)
		{
			surface_renderer::disable_attribute_array_manager(ctx, aam);
			has_extents = false;
			has_secondary_colors = false;
			has_border_colors = false;
			has_border_infos = false;
			has_translations = false;
			has_rotations = false;
			has_depth_offsets = false;
		}
		void rectangle_renderer::set_textured_rectangle(const context& ctx, const textured_rectangle& tcr)
		{
			has_positions = true;
			has_extents = true;
			has_texcoords = true;
			set_position_is_center(false);
			ref_prog().set_attribute(ctx, "position", tcr.rectangle.get_min_pnt());
			ref_prog().set_attribute(ctx, "extent", tcr.rectangle.get_max_pnt());
			ref_prog().set_attribute(ctx, "texcoord", tcr.texcoords);
		}
		
		bool rectangle_renderer::build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines)
		{
			return prog.build_program(ctx, "rectangle.glpr", true, defines);
		}
		bool rectangle_renderer::init(context& ctx)
		{
			bool res = renderer::init(ctx);
			return res;
		}
		void rectangle_renderer::set_position_is_center(bool _position_is_center)
		{
			get_style<rectangle_render_style>().position_is_center = _position_is_center;
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
			int sci = ref_prog().get_attribute_location(ctx, "secondary_color");
			if(!has_secondary_colors && sci != -1)
				ref_prog().set_attribute(ctx, "secondary_color", rrs.default_secondary_color);
			if (!has_border_colors)
				ref_prog().set_attribute(ctx, "border_color", rrs.default_border_color);
			if (!has_border_infos)
				ref_prog().set_attribute(ctx, "border_info", vec3(rrs.border_width_in_pixel, rrs.percentual_border_width, float(rrs.border_mode)));
			// configure opengl
			if (rrs.blend_rectangles) {
				ctx.push_blend_state();
				ctx.enable_blending();
				ctx.set_blend_func_back_to_front();
			}
			ref_prog().set_uniform(ctx, "has_rotations", has_rotations);
			ref_prog().set_uniform(ctx, "has_translations", has_translations);
			ref_prog().set_uniform(ctx, "position_is_center", get_style<rectangle_render_style>().position_is_center);
			ref_prog().set_uniform(ctx, "use_texture", has_texcoords);
			ref_prog().set_uniform(ctx, "viewport_height", (float)ctx.get_height());
			ref_prog().set_uniform(ctx, "texture_mode", (const int&)rrs.texture_mode);
			ref_prog().set_uniform(ctx, "pixel_blend", rrs.pixel_blend);
			ref_prog().set_uniform(ctx, "border_width_in_pixel", rrs.border_width_in_pixel);
			ref_prog().set_uniform(ctx, "percentual_border_width", rrs.percentual_border_width);
			ref_prog().set_uniform(ctx, "border_mode", (const int&)rrs.border_mode);
			return true;
		}
		bool rectangle_renderer::disable(cgv::render::context& ctx)
		{
			if(!attributes_persist()) {
				has_extents = false;
				has_secondary_colors = false;
				has_border_colors = false;
				has_border_infos = false;
				has_rotations = false;
				has_translations = false;
				has_depth_offsets = false;
			}
			const rectangle_render_style& rrs = get_style<rectangle_render_style>();
			if (rrs.blend_rectangles)
				ctx.pop_blend_state();
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
				rh.reflect_member("texture_mode", texture_mode) &&
				rh.reflect_member("blend_rectangles", blend_rectangles) &&
				rh.reflect_member("border_width_in_pixel", border_width_in_pixel) &&
				rh.reflect_member("percentual_border_width", percentual_border_width) &&
				rh.reflect_member("border_mode", border_mode) &&
				rh.reflect_member("default_depth_offset", default_depth_offset) &&
				rh.reflect_member("default_secondary_color", default_secondary_color) &&
				rh.reflect_member("default_border_color", default_border_color);
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

				p->add_member_control(b, "Default Secondary Color", prs_ptr->default_secondary_color);

				p->add_member_control(b, "Default Border Color", prs_ptr->default_border_color);
				p->add_member_control(b, "Border Width in Pixel", prs_ptr->border_width_in_pixel, "value_slider", "min=-10;max=10;ticks=true");
				p->add_member_control(b, "Percentual Border Width", prs_ptr->percentual_border_width, "value_slider", "min=-0.5;max=0.5;ticks=true");
				p->add_member_control(b, "Border Mode", (cgv::type::DummyEnum&)prs_ptr->border_mode, "dropdown",
					"enums='Separate=0,Width,Height,Minimum Width/Height'");
				p->add_member_control(b, "Pixel Blend", prs_ptr->pixel_blend, "value_slider", "min=0.0;max=2;ticks=true");
				p->add_member_control(b, "Texture Mode", (cgv::type::DummyEnum&)prs_ptr->texture_mode, "dropdown",
					"enums='Replace,Replace Alpha,Multiply Color,Multiply Secondary Color,Multiply Border Color,"
					"Mix Color and Secondary Color,Mix Color and Border Color,Mix Secondary Color and Color,Mix Border Color and Color,"
					"Red Mix Color and Secondary Color,Red Mix Color and Border Color,Red Mix Secondary Color and Color,Red Mix Border Color and Color'");
				p->add_member_control(b, "Blend", prs_ptr->blend_rectangles, "toggle");
				p->add_member_control(b, "Default Depth Offset", prs_ptr->default_depth_offset, "value_slider", "min=0.000001;max=0.1;step=0.0000001;log=true;ticks=true");
				p->add_member_control(b, "Position is Center", prs_ptr->position_is_center, "toggle");

				if(p->begin_tree_node("Surface", prs_ptr->use_group_color, false, "level=3")) {
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