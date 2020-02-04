#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		class CGV_API rounded_cone_renderer;

		//! reference to a singleton box renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singelton renderer is destructed. */
		extern CGV_API rounded_cone_renderer& ref_rounded_cone_renderer(context& ctx, int ref_count_change = 0);

		/// boxes use surface render styles
		typedef surface_render_style rounded_cone_render_style;

		/// renderer that supports point splatting
		class CGV_API rounded_cone_renderer : public surface_renderer
		{
		protected:
			///
			vec3 eye_pos;
			/// overload to allow instantiation of rounded_cone_renderer
			render_style* create_render_style() const;
		public:
			struct cone {
				cgv::math::fvec<float, 4U> start, end;
			};
			/// initializes position_is_center to true 
			rounded_cone_renderer();
			void set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr);
			/// set the flag, whether the position is interpreted as the box center, true by default
			void set_eye_position(vec3 eye_position);
			/// construct shader programs and return whether this was successful, call inside of init method of drawable
			bool init(context& ctx);
			/// 
			bool enable(context& ctx);
			/// specify box array directly. This sets position_is_center to false as well as position and extent array
			//template <typename T>
			void set_cone_array(const context& ctx, const std::vector<cone>& cones) {
				set_composed_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "start"),
					&cones.front(), cones.size(), cones[0].start);
				ref_composed_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "end"), 
					ref_prog().get_attribute_location(ctx, "start"), &cones.front(), cones.size(), cones[0].end);
				has_positions = true;
			}
			/// specify box array directly. This sets position_is_center to false as well as position and extent array
			//template <typename T>
			void set_cone_array(const context& ctx, const cone* cones, size_t count) {
				set_composed_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "start"), 
					cones, count, cones[0].start);
				ref_composed_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "end"), 
					ref_prog().get_attribute_location(ctx, "start"), cones, count, cones[0].end);
				has_positions = true;
			}
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool disable(context& ctx);
		};
	}
}

#include <cgv/config/lib_end.h>