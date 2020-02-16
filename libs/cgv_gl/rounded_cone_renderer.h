#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		class CGV_API rounded_cone_renderer;

		//! reference to a singleton rounded cone renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singelton renderer is destructed. */
		extern CGV_API rounded_cone_renderer& ref_rounded_cone_renderer(context& ctx, int ref_count_change = 0);

		/// rounded cones use surface render styles
		//typedef surface_render_style rounded_cone_render_style;

		struct CGV_API rounded_cone_render_style : public surface_render_style
		{	
			/// multiplied to the sphere radii, initialized to 1
			float radius_scale;
			/// default value assigned to radius attribute in \c enable method of rounded cone renderer, set to 1 in constructor
			float radius;

			float ao_offset;
			float ao_distance;
			float ao_strength;

			vec3 tex_offset;
			vec3 tex_scaling;
			vec3 tex_coord_scaling;
			float texel_size;
			float cone_angle_factor;
			std::vector<vec3> sample_dirs;
			/// construct with default values
			rounded_cone_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API rounded_cone_renderer : public surface_renderer
		{
		protected:
			bool has_radii;
			/// overload to allow instantiation of rounded_cone_renderer
			render_style* create_render_style() const;
		public:
			/// initializes position_is_center to true 
			rounded_cone_renderer();
			void set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr);
			/// construct shader programs and return whether this was successful, call inside of init method of drawable
			bool init(context& ctx);
			/// 
			bool enable(context& ctx);
			///
			template <typename T = float>
			void set_radius_array(const context& ctx, const std::vector<T>& radii) { has_radii = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "radius"), radii); }
			/// 
			template <typename T = float>
			void set_radius_array(const context& ctx, const T* radii, size_t nr_elements, unsigned stride_in_bytes = 0) { has_radii = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "radius"), radii, nr_elements, stride_in_bytes); }
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool disable(context& ctx);
		};

		struct CGV_API rounded_cone_render_style_reflect : public rounded_cone_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<rounded_cone_render_style, rounded_cone_render_style_reflect> get_reflection_traits(const rounded_cone_render_style&);
	}
}

#include <cgv/config/lib_end.h>