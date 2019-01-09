#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		
		/** style of a point */
		struct CGV_API sphere_render_style : public surface_render_style
		{
			/*@name sphere rendering attributes*/
			//@{
			/// multiplied to the sphere radii, initialized to 1
			float radius_scale;
			/// default value assigned to radius attribute in \c enable method of sphere renderer, set to 1 in constructor
			float radius;
			/// whether to use the group radius
			bool use_group_radius;
			/// set to 1 in constructor 
			float blend_width_in_pixel;
			/// set to 0 in constructor
			float halo_width_in_pixel;
			/// set to 0 in constructor
			float percentual_halo_width;
			/// color of halo with opacity channel
			cgv::media::color<float, cgv::media::RGB, cgv::media::OPACITY> halo_color;
			/// strength in [0,1] of halo color with respect to color of primitive
			float halo_color_strength;
			//@}

			/// construct with default values
			sphere_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API sphere_renderer : public surface_renderer
		{
		protected:
			bool has_radii;
			bool has_group_radii;
			float y_view_angle;
			/// overload to allow instantiation of point_renderer
			render_style* create_render_style() const;
		public:
			///
			sphere_renderer();
			///
			bool init(context& ctx);
			///
			void set_y_view_angle(float y_view_angle);
			///
			template <typename T = float>
			void set_radius_array(const context& ctx, const std::vector<T>& radii) { has_radii = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "radius"), radii); }
			/// 
			template <typename T = float>
			void set_radius_array(const context& ctx, const T* radii, size_t nr_elements, size_t stride_in_bytes = 0) { has_radii = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "radius"), radii, nr_elements, stride_in_bytes); }
			///
			template <typename T = float>
			void set_group_radii(const context& ctx, const std::vector<T>& group_radii) { has_group_radii = true; ref_prog().set_uniform_array(ctx, "group_radii", group_radii); }
			/// use this function if you store spheres in vec4 with the 4th component the radius
			template <typename T = float>
			void set_sphere_array(const context& ctx, const std::vector<cgv::math::fvec<T, 4> >& spheres) {
				set_position_array(ctx, &(reinterpret_cast<cgv::math::fvec<T, 3>&>(spheres[0]), spheres.size(), sizeof(cgv::math::fvec<T, 4>)));
				set_radius_array(ctx, &spheres[0][3], spheres.size(), sizeof(cgv::math::fvec<T, 4>));
			}
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool enable(context& ctx);
			///
			bool disable(context& ctx);
		};
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			struct CGV_API sphere_render_style : public cgv::render::sphere_render_style
			{
				bool self_reflect(cgv::reflect::reflection_handler& rh);
			};
		}
		extern CGV_API cgv::reflect::extern_reflection_traits<cgv::render::sphere_render_style, cgv::reflect::render::sphere_render_style> get_reflection_traits(const cgv::render::sphere_render_style&);
	}
}

#include <cgv/config/lib_end.h>