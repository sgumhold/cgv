#pragma once

#include "group_renderer.h"
#include <cgv/media/color.h>

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		/** style of a line */
		struct CGV_API line_render_style : public group_render_style
		{
			// default values for program attributes

			/// default normal for case when "normal" attribute is not set
			vec3 default_normal;
			/// default color for case when "color" attribute is not set
			rgba default_color;
			/// default depth offset for case when "depth_offset" attribute is not set
			float default_depth_offset;
			/// default line width for case when "line_width" attribute is not set
			float default_line_width;

			// influence on opengl state
			bool blend_lines;
		protected:
			friend class line_renderer;
			mutable GLboolean is_blend;
			mutable GLint blend_src, blend_dst;
		public:
			// vertex shader uniforms

			/// halo color
			rgba  halo_color;
			/// halo width in pixel
			float halo_width_in_pixel;
			/// halo width in percent of line width
			float percentual_halo_width;
			/// whether to span line splat in screen aligned coordinate system
			bool screen_aligned;

			// geometry shader uniforms

			/// whether to measure line width in pixels - otherwise in eye space relative to reference_line_width
			bool measure_line_width_in_pixel;
			/// reference line width multiplied to line width if measure_line_width_in_pixel is false
			float reference_line_width;
			/// blend with in pixels used for line smoothing
			float blend_width_in_pixel;

			// fragment shader uniforms

			/// parameter in [0,1] to mix line color with halo color
			float halo_color_strength;
			/// construct with default values
			line_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API line_renderer : public group_renderer
		{
		protected:
			bool has_normals;
			bool has_line_widths;
			bool has_depth_offsets;
			/// 
			render_style* create_render_style() const;
			/// build line program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
		public:
			/// construct line rendering
			line_renderer();
			///
			bool enable(context& ctx);
			///
			bool disable(context& ctx);
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			///
			bool init(context& ctx);
			/// specify a single normal for all lines
			template <typename T>
			void set_normal(const context& ctx, const cgv::math::fvec<T,3>& normal) { has_normals = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "normal"), normal); }
			/// templated method to set the normal attribute from a vector of normals of type T, which should have 3 components
			template <typename T>
			void set_normal_array(const context& ctx, const std::vector<T>& normals) { has_normals = true;  set_attribute_array(ctx, "normal", normals); }
			/// templated method to set the normal attribute from an array of normals of type T, which should have 3 components
			template <typename T>
			void set_normal_array(const context& ctx, const T* normals, size_t nr_elements, unsigned stride_in_bytes = 0) { has_normals = true;  set_attribute_array(ctx, "normal", normals, nr_elements, stride_in_bytes); }
			/// specify a single line_width for all lines
			template <typename T>
			void set_line_width(const context& ctx, const T& line_width) { has_line_widths = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "line_width"), line_width); }
			/// line_width array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_line_width_array(const context& ctx, const std::vector<T>& line_widths) { has_line_widths = true;  set_attribute_array(ctx, "line_width", line_widths); }
			/// line_width array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_line_width_array(const context& ctx, const T* line_widths, size_t nr_elements, unsigned stride_in_bytes = 0) { has_line_widths = true;  set_attribute_array(ctx, "line_width", line_widths, nr_elements, stride_in_bytes); }
			/// specify a single depth_offset for all lines
			template <typename T>
			void set_depth_offset(const context& ctx, const T& depth_offset) { has_depth_offsets = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "depth_offset"), depth_offset); }
			/// depth_offset array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_depth_offset_array(const context& ctx, const std::vector<T>& depth_offsets) { has_depth_offsets = true;  set_attribute_array(ctx, "depth_offset", depth_offsets); }
			/// depth_offset array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_depth_offset_array(const context& ctx, const T* depth_offsets, size_t nr_elements, unsigned stride_in_bytes = 0) { has_depth_offsets = true;  set_attribute_array(ctx, "depth_offset", depth_offsets, nr_elements, stride_in_bytes); }
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};
		///	
		extern CGV_API line_renderer& ref_line_renderer(context& ctx, int ref_count_change = 0);
		///
		struct CGV_API line_render_style_reflect : public line_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<line_render_style, line_render_style_reflect> get_reflection_traits(const line_render_style&);
	}
}

#include <cgv/config/lib_end.h>