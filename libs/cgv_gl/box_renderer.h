#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		class CGV_API box_renderer;

		//! reference to a singleton box renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API box_renderer& ref_box_renderer(context& ctx, int ref_count_change = 0);

		/// boxes use surface render styles
		struct CGV_API box_render_style : public surface_render_style
		{
			/// extent used in case extent array is not specified
			vec3 default_extent;
			/// box anchor position relative to center that corresponds to the position attribute
			vec3 relative_anchor;
			/// whether to use rounding of edges and corners (enabling re-compiles shader program)
			bool rounding = false;
			/// radius used in case radius array is not specified
			float default_radius = 0.01f;
			/// default constructor sets default extent to (1,1,1) and relative anchor to (0,0,0)
			box_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API box_renderer : public surface_renderer
		{
		protected:
			/// store whether extent array has been specified
			bool has_extents;
			/// store whether extent array has been specified
			bool has_radii;
			/// whether secondary color or color array was set
			bool has_secondary_colors;
			/// whether array with per box translations has been specified
			bool has_translations;
			/// whether array with per box rotations has been specified
			bool has_rotations;
			/// whether position is box center, if not it is lower left bottom corner
			bool position_is_center;
			/// overload to allow instantiation of box_renderer
			render_style* create_render_style() const;
			/// build box program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
			///
			void update_defines(shader_define_map& defines);
		public:
			/// initializes position_is_center to true 
			box_renderer();
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// set the flag, whether the position is interpreted as the box center, true by default
			void set_position_is_center(bool _position_is_center);
			/// enable box renderer
			bool enable(context& ctx);
			/// specify a single extent for all boxes
			template <typename T>
			void set_extent(const context& ctx, const T& extent) { has_extents = true; ref_prog().set_attribute(ctx, get_prog_attribute_location(ctx, "extent"), extent); }
			/// extent array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_extent_array(const context& ctx, const std::vector<T>& extents) { has_extents = true;  set_attribute_array(ctx, "extent", extents); }
			/// extent array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_extent_array(const context& ctx, const T* extents, size_t nr_elements, unsigned stride_in_bytes = 0) { has_extents = true;  set_attribute_array(ctx, "extent", extents, nr_elements, stride_in_bytes); }
			///
			template <typename T = float>
			void set_radius(const context& ctx, const T& radius) { has_radii = true; ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "radius"), radius); }
			///
			template <typename T = float>
			void set_radius_array(const context& ctx, const std::vector<T>& radii) { has_radii = true; set_attribute_array(ctx, "radius", radii); }
			/// 
			template <typename T = float>
			void set_radius_array(const context& ctx, const T* radii, size_t nr_elements, unsigned stride_in_bytes = 0) { has_radii = true;  set_attribute_array(ctx, "radius", radii, nr_elements, stride_in_bytes); }
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
			/// specify a single box. This sets position_is_center to false as well as position and extent attributes
			template <typename T>
			void set_box(const context& ctx, const cgv::media::axis_aligned_box<T, 3>& box) {
				set_position(ctx, box.get_min_pnt());
				set_extent(ctx, box.get_max_pnt());
				set_position_is_center(false);
			}
			/// specify box array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_box_array(const context& ctx, const std::vector<cgv::media::axis_aligned_box<T, 3> >& boxes) {
				set_composed_attribute_array(ctx, "position", &boxes.front(), boxes.size(), boxes[0].get_min_pnt());
				ref_composed_attribute_array(ctx, "extent", "position", &boxes.front(), boxes.size(), boxes[0].get_max_pnt());
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
			}
			/// specify box array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_box_array(const context& ctx, const cgv::media::axis_aligned_box<T, 3>* boxes, size_t count) {
				set_composed_attribute_array(ctx, "position", boxes, count, boxes[0].get_min_pnt());
				ref_composed_attribute_array(ctx, "extent", "position", boxes, count, boxes[0].get_max_pnt());
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
			}
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const std::vector<T>& translations) { has_translations = true; set_attribute_array(ctx, "translation", translations); }
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const T* translations, size_t nr_elements, unsigned stride_in_bytes = 0) { has_translations = true; set_attribute_array(ctx, "translation", translations, nr_elements, stride_in_bytes); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const std::vector<T>& rotations) { has_rotations = true; set_attribute_array(ctx, "rotation", rotations); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const T* rotations, size_t nr_elements, unsigned stride_in_bytes = 0) { has_rotations = true; set_attribute_array(ctx, "rotation", rotations, nr_elements, stride_in_bytes); }
			///
			bool disable(context& ctx);
			///
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};
		struct CGV_API box_render_style_reflect : public box_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<box_render_style, box_render_style_reflect> get_reflection_traits(const box_render_style&);
	}
}

#include <cgv/config/lib_end.h>