#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		class CGV_API arrow_renderer;

		//! reference to a singleton surfel renderer that can be shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API arrow_renderer& ref_arrow_renderer(context& ctx, int ref_count_change = 0);

		/// <summary>
		/// different modes to compute the head length of an arrow
		/// </summary>
		enum ArrowHeadLengthMode
		{
			AHLM_RELATIVE_TO_RADIUS = 1, ///
			AHLM_RELATIVE_TO_LENGTH = 2,///
			AHLM_MINIMUM_OF_RADIUS_AND_LENGTH = 3///
		};

		/** style of a point */
		struct CGV_API arrow_render_style : public surface_render_style
		{
			/*@name arrow rendering attributes*/
			//@{
			/// smallest value for the radius
			float radius_lower_bound;
			/// 
			float radius_relative_to_length;
			/// scaling factor of head radius with respect to tail radius
			float head_radius_scale;
			///
			ArrowHeadLengthMode head_length_mode;
			/// 
			float head_length_relative_to_radius;
			/// 
			float head_length_relative_to_length;
			///
			float length_scale;
			///
			float color_scale;
			///
			bool normalize_length;
			/// 
			float relative_location_of_position;
			///
			float length_eps;
			//@}
			/// construct with default values
			arrow_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API arrow_renderer : public surface_renderer
		{
		protected:
			bool has_directions;
			bool position_is_center;
			bool direction_is_end_point;
			/// overload to allow instantiation of arrow_renderer
			render_style* create_render_style() const;
			/// build arrow program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
		public:
			///
			arrow_renderer();
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// templated method to set the direction attribute from a vector of directions of type T, which should have 3 components
			template <typename T>
			void set_direction_array(const context& ctx, const std::vector<T>& directions) { has_directions = true;  direction_is_end_point = false;  set_attribute_array(ctx, "direction", directions); }
			/// templated method to set the direction attribute from an array of directions of type T, which should have 3 components
			template <typename T>
			void set_direction_array(const context& ctx, const T* directions, size_t nr_elements, unsigned stride_in_bytes = 0) { has_directions = true; direction_is_end_point = false; set_attribute_array(ctx, "direction", directions, nr_elements, stride_in_bytes); }
			/// templated method to set the end_point attribute from a vector of end_points of type T, which should have 3 components
			template <typename T>
			void set_end_point_array(const context& ctx, const std::vector<T>& end_points) { has_directions = true;  direction_is_end_point = true;  set_attribute_array(ctx, "direction", end_points); }
			/// templated method to set the end_point attribute from an array of end_points of type T, which should have 3 components
			template <typename T>
			void set_end_point_array(const context& ctx, const T* end_points, size_t nr_elements, unsigned stride_in_bytes = 0) { has_directions = true; direction_is_end_point = true; set_attribute_array(ctx, "direction", end_points, nr_elements, stride_in_bytes); }
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
		struct CGV_API arrow_render_style_reflect : public arrow_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<arrow_render_style, arrow_render_style_reflect> get_reflection_traits(const arrow_render_style&);
	}
}

#include <cgv/config/lib_end.h>