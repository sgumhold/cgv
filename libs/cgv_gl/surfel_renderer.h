#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		
		/** style of a point */
		struct CGV_API surfel_render_style : public surface_render_style
		{
			/*@name point rendering attributes*/
			//@{
			/// default value assigned to point size attribute in \c enable method of point renderer, set to 1 in constructor
			float point_size;
			/// whether to use the 
			bool use_group_point_size;
			/// whether to measure point size in pixels or in world space relative to reference_pixel_size passed to enable method, defaults to true
			bool measure_point_size_in_pixel;
			//@}

			/*@name global point rendering options*/
			//@{
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
			/// set to true in constructor
			bool orient_splats;
			/// set to false in constructor
			bool blend_points;
			/// construct with default values
			surfel_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API surfel_renderer : public surface_renderer
		{
		protected:
			bool has_point_sizes;
			bool has_group_point_sizes;
			bool has_indexed_colors;
			float reference_point_size;
			float y_view_angle;
			/// overload to allow instantiation of surfel_renderer
			render_style* create_render_style() const;
		public:
			///
			surfel_renderer();
			///
			bool init(context& ctx);
			///
			void set_reference_point_size(float _reference_point_size);
			///
			void set_y_view_angle(float y_view_angle);
			///
			template <typename T = float>
			void set_point_size_array(const context& ctx, const std::vector<T>& point_sizes) { has_point_sizes = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "point_size"), point_sizes); }
			///
			template <typename T = unsigned, typename C = cgv::media::color<float,cgv::media::RGB,cgv::media::OPACITY> >
			void set_indexed_color_array(const context& ctx, const std::vector<T>& color_indices, const std::vector<C>& palette) {
				has_indexed_colors = true; 
				set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "color_index"), color_indices); 
				ref_prog().set_uniform_array(ctx, "palette", palette); 
			}
			///
			template <typename T = float>
			void set_group_point_sizes(const context& ctx, const std::vector<T>& group_point_sizes) { has_group_point_sizes = true; ref_prog().set_uniform_array(ctx, "group_point_sizes", group_point_sizes); }
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
			struct CGV_API surfel_render_style : public cgv::render::surfel_render_style
			{
				bool self_reflect(cgv::reflect::reflection_handler& rh);
			};
		}
		extern CGV_API cgv::reflect::extern_reflection_traits<cgv::render::surfel_render_style, cgv::reflect::render::surfel_render_style> get_reflection_traits(const cgv::render::surfel_render_style&);
	}
}

#include <cgv/config/lib_end.h>