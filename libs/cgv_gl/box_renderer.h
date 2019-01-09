#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		/// renderer that supports point splatting
		class CGV_API box_renderer : public surface_renderer
		{
		protected:
			/// store whether extent array has been specified is specified
			bool has_extents;
			/// whether array with per box translations has been specified
			bool has_translations;
			/// whether array with per box rotations has been specified
			bool has_rotations;
			/// whether position is box center, if not it is lower left bottom corner
			bool position_is_center;
			/// overload to allow instantiation of box_renderer
			render_style* create_render_style() const;
		public:
			/// initializes position_is_center to true 
			box_renderer();
			/// set the flag, whether the position is interpreted as the box center, true by default
			void set_position_is_center(bool _position_is_center);
			/// construct shader programs and return whether this was successful, call inside of init method of drawable
			bool init(context& ctx);
			/// 
			bool enable(context& ctx);
			/// extent array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_extent_array(const context& ctx, const std::vector<T>& extents) { has_extents = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"), extents); }
			/// extent array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_extent_array(const context& ctx, const T* extents, size_t nr_elements, size_t stride_in_bytes = 0) { has_extents = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"), extents, nr_elements, stride_in_bytes); }
			/// specify box array directly. This sets position_is_center to false as well as position and extent array
			template <typename T>
			void set_box_array(const context& ctx, const std::vector<cgv::media::axis_aligned_box<T, 3> >& boxes) {
				set_position_is_center(false);
				set_position_array(ctx, &boxes[0].get_min_pnt(), boxes.size(), sizeof(cgv::media::axis_aligned_box<T, 3>));
				set_extent_array(ctx, &boxes[0].get_max_pnt(), boxes.size(), sizeof(cgv::media::axis_aligned_box<T, 3>));
			}
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const std::vector<T>& translations) { has_translations = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "translation"), translations); }
			/// template method to set the translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_translation_array(const context& ctx, const T* translations, size_t nr_elements, size_t stride) { has_translations = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "translation"), translations, nr_elements, stride); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const std::vector<T>& rotations) { has_rotations = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "rotation"), rotations); }
			/// template method to set the rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_rotation_array(const context& ctx, const T* rotations, size_t nr_elements, size_t stride) { has_rotations = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "rotation"), rotations, nr_elements, stride); }
			///
			bool validate_attributes(const context& ctx);
			///
			bool disable(context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>