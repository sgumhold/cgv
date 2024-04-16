#pragma once

#include "surface_renderer.h"
#include <cgv/reflect/reflect_enum.h>
#include <cgv/render/textured_rectangle.h>

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		class CGV_API rectangle_renderer;

		//! reference to a singleton plane renderer that can be shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API rectangle_renderer& ref_rectangle_renderer(context& ctx, int ref_count_change = 0);

		/// different modes to use texture during rectangle rendering
		enum RectangleTextureMode
		{
			RTM_REPLACE = 0,                       // result   = texture
			RTM_REPLACE_ALPHA,                     // result   = vec4(color.rgb, texture.r)
			RTM_MULTIPLY_COLOR,                    // result   = color * texture
			RTM_MULTIPLY_SECONDARY_COLOR,          // result   = secondary_color * texture
			RTM_MULTIPLY_BORDER_COLOR,             // result   = border_color * texture
			RTM_MIX_COLOR_AND_SECONDARY_COLOR,     // result   = mix(color,secondary_color,texture)
			RTM_MIX_COLOR_AND_BORDER_COLOR,        // result   = mix(color,border_color,texture)
			RTM_MIX_SECONDARY_COLOR_AND_COLOR,     // result   = mix(secondary_color,color,texture)
			RTM_MIX_BORDER_COLOR_AND_COLOR,        // result   = mix(border_color,color,texture)
			RTM_RED_MIX_COLOR_AND_SECONDARY_COLOR, // result   = mix(color,secondary_color,texture.r)
			RTM_RED_MIX_COLOR_AND_BORDER_COLOR,    // result   = mix(color,border_color,texture.r)
			RTM_RED_MIX_SECONDARY_COLOR_AND_COLOR, // result   = mix(secondary_color,color,texture.r)
			RTM_RED_MIX_BORDER_COLOR_AND_COLOR     // result   = mix(border_color,color,texture.r)
		};
		/// different modes to compute relative border with for rectangle rendering
		enum RectangleBoderMode
		{
			RBM_SEPARATE = 0, // border_width.xy = percentual_border_width*extent.xy
			RBM_WIDTH,        // border_width.xy = vec2(percentual_border_width*extent.x)
			RBM_HEIGHT,       // border_width.xy = vec2(percentual_border_width*extent.y)
			RBM_MIN           // border_width.xy = vec2(percentual_border_width*min(extent.x, extent.y))
		};
		/// allow to use RectangleTextureMode in self reflection
		extern CGV_API cgv::reflect::enum_reflection_traits<RectangleTextureMode> get_reflection_traits(const RectangleTextureMode&);
		/// allow to use RectangleBoderMode in self reflection
		extern CGV_API cgv::reflect::enum_reflection_traits<RectangleBoderMode> get_reflection_traits(const RectangleBoderMode&);
		/// <summary>
		/// configuration of rectangle renderer
		/// </summary>
		struct CGV_API rectangle_render_style : public surface_render_style
		{
			//! flag whether position attribute is the rectangle center; otherwise position is lower left corner (default: true)
			/*! This is the only member of style that can be set by rectangle_renderer. 
			    The following functions set this member to \c false:
				- set_rectangle()
				- set_rectangle_array()
				- set_textured_rectangle_array()
				Thus it is important to set the render style of the renderer before you call any of these functions.*/
			mutable bool position_is_center;
			/// default value of secondary color which is ignored if set_secondary_color_array() is used to set per rectangle secondary colors (default: opaque 50% grey)
			rgba default_secondary_color;
			/// default value for the border color attribute which is ignored when set_border_color_array() is used to set per rectangle border colors (default: opaque black)
			rgba default_border_color;
			/// border width measured in pixels (default: 0)
			float border_width_in_pixel;
			/// border width measured relative to rectangle extent computed according to current \c border_mode (default: 0)
			float percentual_border_width;
			/// different modes of computing the width of the border (default: RBM_MIN)
			RectangleBoderMode border_mode;
			/// number of pixels around the rectangle splat used for antialiasing (default: 0.0f)
			float pixel_blend;
			/// mode of using texture during rastrization (default: RTM_REPLACE)
			RectangleTextureMode texture_mode;
			//! default depth offset added to depth value of fragment. (default: 0.0f)
			/*! Depth values are in [0,1]. Minimal depth offsets can be estimated from 1/2^n where n is
				the number of bits in the depth buffer (typically 24 or 32). */
			float default_depth_offset;
			/// if true the renderer enables blending in the enable method and recovers previous blending mode on disable (default: false)
			bool blend_rectangles;
			/// default constructor initializes members as specified in member comments
			rectangle_render_style();
		};

		/// renderer that supports plane rendering
		class CGV_API rectangle_renderer : public surface_renderer
		{
		protected:
			/// whether extent array has been specified
			bool has_extents;
			/// whether secondary color or color array was set
			bool has_secondary_colors;
			/// whether border color or color array was set
			bool has_border_colors;
			/// whether border info or info array was set
			bool has_border_infos;
			/// whether translation array has been specified
			bool has_translations;
			/// whether rotation array has been specified
			bool has_rotations;
			/// whether depth offset array has been specified
			bool has_depth_offsets;
			float y_view_angle;
			/// overload to allow instantiation of rectangle_renderer
			render_style* create_render_style() const;
			/// build rectangle program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
		public:
			///
			rectangle_renderer();
			///
			void set_y_view_angle(float y_view_angle);
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			///
			bool init(context& ctx);
			/// set the flag of the render style, whether the position is interpreted as the box center
			void set_position_is_center(bool _position_is_center);
			/// specify a single extent for all boxes
			template <typename T>
			void set_extent(const context& ctx, const cgv::math::fvec<T, 2U>& extent) { has_extents = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "extent"), extent); }
			/// extent array specifies plane side lengths from origin to edge
			template <typename T>
			void set_extent_array(const context& ctx, const std::vector<cgv::math::fvec<T, 2U>>& extents) { has_extents = true;  set_attribute_array(ctx, "extent", extents); }
			/// extent array specifies plane side lengths from origin to edge
			template <typename T>
			void set_extent_array(const context& ctx, const cgv::math::fvec<T, 2U>* extents, size_t nr_elements, unsigned stride_in_bytes = 0) { has_extents = true;  set_attribute_array(ctx, "extent", extents, nr_elements, stride_in_bytes); }
			/// specify a single rectangle without array. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_rectangle(const context& ctx, const cgv::media::axis_aligned_box<T, 2>& box) {
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
				ref_prog().set_attribute(ctx, "position", box.get_min_pnt());
				ref_prog().set_attribute(ctx, "extent", box.get_max_pnt());
			}
			/// specify rectangle array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_rectangle_array(const context& ctx, const std::vector<cgv::media::axis_aligned_box<T, 2> >& boxes) {
				set_composed_attribute_array(ctx, "position", &boxes.front(), boxes.size(), boxes[0].get_min_pnt());
				ref_composed_attribute_array(ctx, "extent", "position", &boxes.front(), boxes.size(), boxes[0].get_max_pnt());
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
			}
			/// specify ractangle array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_rectangle_array(const context& ctx, const cgv::media::axis_aligned_box<T, 2>* boxes, size_t count) {
				set_composed_attribute_array(ctx, "position", boxes, count, boxes[0].get_min_pnt());
				ref_composed_attribute_array(ctx, "extent", "position", boxes, count, boxes[0].get_max_pnt());
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
			}
			/// specify rectangle without array. This sets position_is_center to false as well as position and extent array
			void set_textured_rectangle(const context& ctx, const textured_rectangle& tcr);
			/// specify rectangle array directly. This sets position_is_center to false as well as position and extent array
			void set_textured_rectangle_array(const context& ctx, const std::vector<textured_rectangle>& tc_rects) {
				set_composed_attribute_array(ctx, "position", &tc_rects.front(), tc_rects.size(), tc_rects[0].rectangle.get_min_pnt());
				ref_composed_attribute_array(ctx, "extent", "position", &tc_rects.front(), tc_rects.size(), tc_rects[0].rectangle.get_max_pnt());
				ref_composed_attribute_array(ctx, "texcoord", "position", &tc_rects.front(), tc_rects.size(), tc_rects[0].texcoords);
				has_positions = true;
				has_extents = true;
				has_texcoords = true;
				set_position_is_center(false);
			}
			/// specify ractangle array directly. This sets position_is_center to false as well as position and extent array
			void set_textured_rectangle_array(const context& ctx, const textured_rectangle* tc_rects, size_t count) {
				set_composed_attribute_array(ctx, "position", tc_rects, count, tc_rects[0].rectangle.get_min_pnt());
				ref_composed_attribute_array(ctx, "extent", "position", tc_rects, count, tc_rects[0].rectangle.get_max_pnt());
				ref_composed_attribute_array(ctx, "texcoord", "position", tc_rects, count, tc_rects[0].texcoords);
				has_positions = true;
				has_extents = true;
				has_texcoords = true;
				set_position_is_center(false);
			}
			/// templated method to set the secondary color attribute from a single color of type T
			template <typename T>
			void set_secondary_color(const context& ctx, const T& color) { has_secondary_colors = true; ref_prog().set_attribute(ctx, "secondary_color", color); }
			/// template method to set the secondary color attribute from a vector of colors of type T
			template <typename T>
			void set_secondary_color_array(const context& ctx, const std::vector<T>& colors) { has_secondary_colors = true;  set_attribute_array(ctx, "secondary_color", colors); }
			/// template method to set the secondary color attribute from a vector of colors of type T
			template <typename T>
			void set_secondary_color_array(const context& ctx, const T* colors, size_t nr_elements, unsigned stride_in_bytes = 0) { has_secondary_colors = true;  set_attribute_array(ctx, "secondary_color", colors, nr_elements, stride_in_bytes); }
			/// method to set the secondary color attribute from a vertex buffer object, the element type must be given as explicit template parameter
			void set_secondary_color_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0);
			/// template method to set the secondary color attribute from a vertex buffer object, the element type must be given as explicit template parameter
			template <typename T>
			void set_secondary_color_array(const context& ctx, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0) { set_secondary_color_array(ctx, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(T()), true), vbo, offset_in_bytes, nr_elements, stride_in_bytes); }

			/// templated method to set the border color attribute from a single color of type T
			template <typename T>
			void set_border_color(const context& ctx, const T& color) { has_border_colors = true; ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "border_color"), color); }
			/// template method to set the border color attribute from a vector of colors of type T
			template <typename T>
			void set_border_color_array(const context& ctx, const std::vector<T>& colors) { has_border_colors = true;  set_attribute_array(ctx, "border_color", colors); }
			/// template method to set the border color attribute from a vector of colors of type T
			template <typename T>
			void set_border_color_array(const context& ctx, const T* colors, size_t nr_elements, unsigned stride_in_bytes = 0) { has_border_colors = true;  set_attribute_array(ctx, "border_color", colors, nr_elements, stride_in_bytes); }
			/// method to set the border color attribute from a vertex buffer object, the element type must be given as explicit template parameter
			void set_border_color_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0);
			/// template method to set the border color attribute from a vertex buffer object, the element type must be given as explicit template parameter
			template <typename T>
			void set_border_color_array(const context& ctx, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0) { set_border_color_array(ctx, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(T()), true), vbo, offset_in_bytes, nr_elements, stride_in_bytes); }
			/// specify a single border_info for all lines
			template <typename T>
			void set_border_info(const context& ctx, const cgv::math::fvec<T, 3>& border_info) { has_border_infos = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "border_info"), border_info); }
			/// templated method to set the border_info attribute array from a vector of border_infos of type T, which should have 3 components
			template <typename T>
			void set_border_info_array(const context& ctx, const std::vector<T>& border_infos) { has_border_infos = true;  set_attribute_array(ctx, "border_info", border_infos); }
			/// templated method to set the border_info attribute from an array of border_infos of type T, which should have 3 components
			template <typename T>
			void set_border_info_array(const context& ctx, const T* border_infos, size_t nr_elements, unsigned stride_in_bytes = 0) { has_border_infos = true;  set_attribute_array(ctx, "border_info", border_infos, nr_elements, stride_in_bytes); }
			/// method to set the border_info attribute from a vertex buffer object, the element type must be given as explicit template parameter
			void set_border_info_array(const context& ctx, type_descriptor element_type, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0);
			/// template method to set the border_info attribute from a vertex buffer object, the element type must be given as explicit template parameter
			template <typename T>
			void set_border_info_array(const context& ctx, const vertex_buffer& vbo, size_t offset_in_bytes, size_t nr_elements, unsigned stride_in_bytes = 0) { set_border_info_array(ctx, type_descriptor(element_descriptor_traits<T>::get_type_descriptor(T()), true), vbo, offset_in_bytes, nr_elements, stride_in_bytes); }

			/// specify a single depth_offset for all lines
			template <typename T>
			void set_depth_offset(const context& ctx, const T& depth_offset) { has_depth_offsets = true;  ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "depth_offset"), depth_offset); }
			/// set per rectangle depth offsets
			template <typename T = float>
			void set_depth_offset_array(const context& ctx, const std::vector<T>& depth_offsets) { has_depth_offsets = true; set_attribute_array(ctx, "depth_offset", depth_offsets); }
			/// template method to set translation for all rectangles from a vector type T, which should have 3 components
			template <typename T>
			void set_translation(const context& ctx, const T& translation) { has_translations = true; ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "translation"), translation); }
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const std::vector<T>& translations) { has_translations = true; set_attribute_array(ctx, "translation", translations); }
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const T* translations, size_t nr_elements, unsigned stride) { has_translations = true; set_attribute_array(ctx, "translation", translations, nr_elements, stride); }
			/// set single rotation for all rectangles from a quaternion of type T, which has 4 components
			template <typename T>
			void set_rotation(const context& ctx, const T& rotation) { has_rotations = true; ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "rotation"), rotation); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const std::vector<T>& rotations) { has_rotations = true; set_attribute_array(ctx, "rotation", rotations); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const T* rotations, size_t nr_elements, unsigned stride = 0) { has_rotations = true; set_attribute_array(ctx, "rotation", rotations, nr_elements, stride); }
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool enable(context& ctx);
			///
			bool disable(context& ctx);
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};
		struct CGV_API rectangle_render_style_reflect : public rectangle_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<rectangle_render_style, rectangle_render_style_reflect> get_reflection_traits(const rectangle_render_style&);
	}
}

#include <cgv/config/lib_end.h>