#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		class CGV_API rectangle_renderer;

		//! reference to a singleton plane renderer that can be shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singelton renderer is destructed. */
		extern CGV_API rectangle_renderer& ref_rectangle_renderer(context& ctx, int ref_count_change = 0);

		typedef surface_render_style plane_render_style;
		
		/// renderer that supports plane rendering
		class CGV_API rectangle_renderer : public surface_renderer
		{
		protected:
			/// whether extent array has been specified
			bool has_extents;
			/// whether translation array has been specified
			bool has_translations;
			/// whether rotation array has been specified
			bool has_rotations;
			/// whether position is rectangle center, if not it is lower left corner
			bool position_is_center;
			/// overload to allow instantiation of rectangle_renderer
			render_style* create_render_style() const;
		public:
			///
			rectangle_renderer();
			///
			void set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr);
			///
			bool init(context& ctx);
			/// set the flag, whether the position is interpreted as the box center, true by default
			void set_position_is_center(bool _position_is_center);
			/// specify a single extent for all boxes
			template <typename T>
			void set_extent(const context& ctx, const cgv::math::fvec<T, 2U>& extent) { has_extents = true;  ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "extent"), extent); }
			/// extent array specifies plane side lengths from origin to edge
			template <typename T>
			void set_extent_array(const context& ctx, const std::vector<cgv::math::fvec<T, 2U>>& extents) { has_extents = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"), extents); }
			/// extent array specifies plane side lengths from origin to edge
			template <typename T>
			void set_extent_array(const context& ctx, const cgv::math::fvec<T, 2U>* extents, size_t nr_elements, unsigned stride_in_bytes = 0) { has_extents = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"), extents, nr_elements, stride_in_bytes); }
			/// specify rectangle array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_rectangle_array(const context& ctx, const std::vector<cgv::media::axis_aligned_box<T, 2> >& boxes) {
				set_composed_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "position"),
					&boxes.front(), boxes.size(), boxes[0].get_min_pnt());
				ref_composed_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"),
					ref_prog().get_attribute_location(ctx, "position"), &boxes.front(), boxes.size(), boxes[0].get_max_pnt());
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
			}
			/// specify ractangle array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_rectangle_array(const context& ctx, const cgv::media::axis_aligned_box<T, 2>* boxes, size_t count) {
				set_composed_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "position"),
					boxes, count, boxes[0].get_min_pnt());
				ref_composed_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"),
					ref_prog().get_attribute_location(ctx, "position"), boxes, count, boxes[0].get_max_pnt());
				has_positions = true;
				has_extents = true;
				set_position_is_center(false);
			}
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const std::vector<T>& translations) { has_translations = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "translation"), translations); }
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const T* translations, size_t nr_elements, unsigned stride) { has_translations = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "translation"), translations, nr_elements, stride); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const std::vector<T>& rotations) { has_rotations = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "rotation"), rotations); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const T* rotations, size_t nr_elements, unsigned stride = 0) { has_rotations = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "rotation"), rotations, nr_elements, stride); }
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
	}
}

#include <cgv/config/lib_end.h>