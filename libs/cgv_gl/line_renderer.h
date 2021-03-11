#pragma once

#include "group_renderer.h"
#include <cgv/media/color.h>

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		/** style of a line */
		struct CGV_API line_render_style : public group_render_style
		{
			float line_width;
			rgb line_color;
			line_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API line_renderer : public group_renderer
		{
		protected:
			bool has_line_widths;
		public:
			/// construct line rendering
			line_renderer();
			///
			bool enable(context& ctx);
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// specify a single line_width for all boxes
			template <typename T>
			void set_line_width(const context& ctx, const T& line_width) { has_line_widths = true;  ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "line_width"), line_width); }
			/// line_width array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_line_width_array(const context& ctx, const std::vector<T>& line_widths) { has_line_widths = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "line_width"), line_widths); }
			/// line_width array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_line_width_array(const context& ctx, const T* line_widths, size_t nr_elements, unsigned stride_in_bytes = 0) { has_line_widths = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "line_width"), line_widths, nr_elements, stride_in_bytes); }
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};
	
		struct CGV_API line_render_style_reflect : public line_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<line_render_style, line_render_style_reflect> get_reflection_traits(const line_render_style&);
	}
}

#include <cgv/config/lib_end.h>