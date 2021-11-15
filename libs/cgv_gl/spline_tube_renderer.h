#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		class CGV_API spline_tube_renderer;

		//! reference to a singleton spline tube renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API spline_tube_renderer& ref_spline_tube_renderer(context& ctx, int ref_count_change = 0);

		struct CGV_API spline_tube_render_style : public surface_render_style
		{	
			/// multiplied to the tube radius, initialized to 1
			float radius_scale;
			/// default tube radius, initialized to 1
			float radius;
			/// construct with default values
			spline_tube_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API spline_tube_renderer : public surface_renderer
		{
		protected:
			/// whether radii are specified
			bool has_radii;
			/// whether tangents are specified
			bool has_tangents;

			vec3 eye_pos;

			/// overload to allow instantiation of spline_tube_renderer
			render_style* create_render_style() const;
			/// build spline tube program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
		public:


			void set_eye_pos(vec3 ep) { eye_pos = ep; }


			/// initializes position_is_center to true 
			spline_tube_renderer();
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			///
			template <typename T = float>
			void set_radius_array(const context& ctx, const std::vector<T>& radii) { has_radii = true; set_attribute_array(ctx, "radius", radii); }
			/// 
			template <typename T = float>
			void set_radius_array(const context& ctx, const T* radii, size_t nr_elements, unsigned stride_in_bytes = 0) { has_radii = true; set_attribute_array(ctx, "radius", radii, nr_elements, stride_in_bytes); }
			///
			template <typename T = float>
			void set_tangent_array(const context& ctx, const std::vector<T>& tangents) { has_tangents = true; set_attribute_array(ctx, "tangent", tangents); }
			/// 
			template <typename T = float>
			void set_tangent_array(const context& ctx, const T* tangents, size_t nr_elements, unsigned stride_in_bytes = 0) { has_tangents = true; set_attribute_array(ctx, "tangent", tangents, nr_elements, stride_in_bytes); }
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

		struct CGV_API spline_tube_render_style_reflect : public spline_tube_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<spline_tube_render_style, spline_tube_render_style_reflect> get_reflection_traits(const spline_tube_render_style&);
	}
}

#include <cgv/config/lib_end.h>