#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		/// renderer that supports point splatting
		class CGV_API box_renderer : public surface_renderer
		{
		protected:
			bool has_extents;
			/// overload to allow instantiation of box_renderer
			render_style* create_render_style() const;
		public:
			box_renderer();
			/// construct shader programs and return whether this was successful, call inside of init method of drawable
			bool init(context& ctx);
			template <typename T>
			void set_extent_array(context& ctx, const std::vector<T>& extents) { has_extents = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"), extents); }
			template <typename T>
			void set_extent_array(context& ctx, const T* extents, size_t nr_elements, size_t stride_in_bytes = 0) { has_extents = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "extent"), extents, nr_elements, stride_in_bytes); }
			bool validate_attributes(context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>