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
			vec3 default_normal = { 0.0f, 0.0f, 1.0f };
			/// default color for case when "color" attribute is not set
			rgba default_color = { 1.0f };
			/// default depth offset for case when "depth_offset" attribute is not set
			float default_depth_offset = 0.0f;
			/// default line width for case when "line_width" attribute is not set
			float default_line_width = 1.0f;

			/// whether to enable blending while rendering (needed for smooth edges; true by default)
			bool blend_lines = false;

			// vertex shader uniforms

			/// halo color
			rgba halo_color = { 0.0f, 0.0f, 0.0f, 1.0f };
			/// halo width in pixel
			float halo_width_in_pixel = 0.0f;
			/// halo width in percent of line width
			float percentual_halo_width = 0.0f;
			/// whether to span line splat in screen aligned coordinate system
			bool screen_aligned = true;

			// geometry shader uniforms

			/// whether to measure line width in pixels - otherwise in eye space relative to reference_line_width
			bool measure_line_width_in_pixel = true;
			/// reference line width multiplied to line width if measure_line_width_in_pixel is false
			float reference_line_width = 0.001f;
			/// blend with in pixels used for line smoothing
			float blend_width_in_pixel = 0.0f;

			// fragment shader uniforms

			/// parameter in [0,1] to mix line color with halo color
			float halo_color_strength = 1.0f;
		};

		/// renderer that supports point splatting
		class CGV_API line_renderer : public group_renderer
		{
		protected:
			bool has_normals = false;
			bool has_line_widths = false;
			bool has_depth_offsets = false;
			/// return the default shader program name
			std::string get_default_prog_name() const override { return "line.glpr"; }
			/// create and return the default render style
			render_style* create_render_style() const override { return new line_render_style(); }
		public:
			///
			bool enable(context& ctx) override;
			///
			bool disable(context& ctx) override;
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam) override;
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam) override;
			///
			bool init(context& ctx) override;
			/// specify a single normal for all lines
			template <typename T>
			void set_normal(const context& ctx, const cgv::math::fvec<T,3>& normal) { has_normals = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "normal"), normal); }
			/// templated method to set the normal attribute from a vector of normals of type T, which should have 3 components
			template <typename T>
			void set_normal_array(const context& ctx, const std::vector<T>& normals) { has_normals = true;  set_attribute_array(ctx, "normal", normals); }
			/// templated method to set the normal attribute from an array of normals of type T, which should have 3 components
			template <typename T>
			void set_normal_array(const context& ctx, const T* normals, size_t nr_elements, unsigned stride_in_bytes = 0) { has_normals = true;  set_attribute_array(ctx, "normal", normals, nr_elements, stride_in_bytes); }
			/// remove the normal attribute
			void remove_normal_array(const context& ctx);
			/// specify a single line_width for all lines
			template <typename T>
			void set_line_width(const context& ctx, const T& line_width) { has_line_widths = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "line_width"), line_width); }
			/// line_width array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_line_width_array(const context& ctx, const std::vector<T>& line_widths) { has_line_widths = true;  set_attribute_array(ctx, "line_width", line_widths); }
			/// line_width array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_line_width_array(const context& ctx, const T* line_widths, size_t nr_elements, unsigned stride_in_bytes = 0) { has_line_widths = true;  set_attribute_array(ctx, "line_width", line_widths, nr_elements, stride_in_bytes); }
			/// remove the line width attribute
			void remove_line_width_array(const context& ctx);
			/// specify a single depth_offset for all lines
			template <typename T>
			void set_depth_offset(const context& ctx, const T& depth_offset) { has_depth_offsets = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "depth_offset"), depth_offset); }
			/// depth_offset array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_depth_offset_array(const context& ctx, const std::vector<T>& depth_offsets) { has_depth_offsets = true;  set_attribute_array(ctx, "depth_offset", depth_offsets); }
			/// depth_offset array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_depth_offset_array(const context& ctx, const T* depth_offsets, size_t nr_elements, unsigned stride_in_bytes = 0) { has_depth_offsets = true;  set_attribute_array(ctx, "depth_offset", depth_offsets, nr_elements, stride_in_bytes); }
			/// remove the depth offset attribute
			void remove_depth_offset_array(const context& ctx);
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
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