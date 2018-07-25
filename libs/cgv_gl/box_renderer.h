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
			box_renderer();
			/// set the flag, whether the position is interpreted as the box center
			void set_position_is_center(bool _position_is_center);
			/// construct shader programs and return whether this was successful, call inside of init method of drawable
			bool init(context& ctx);
			/// 
			bool enable(context& ctx);
			/// extent array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_extent_array(context& ctx, const std::vector<T>& extents) { has_extents = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"), extents); }
			/// extent array specifies box extends in case of position_is_center=true, otherwise the maximum point of each box
			template <typename T>
			void set_extent_array(context& ctx, const T* extents, size_t nr_elements, size_t stride_in_bytes = 0) { has_extents = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"), extents, nr_elements, stride_in_bytes); }
			bool validate_attributes(context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>