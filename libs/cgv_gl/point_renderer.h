#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		
		/** style of a point */
		struct CGV_API point_render_style : public surface_render_style
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
			float outline_width_from_pixel;
			/// set to 0 in constructor
			float percentual_outline_width;
			/// set to true in constructor
			bool smooth_points;
			/// set to true in constructor
			bool orient_splats;
			/// set to false in constructor
			bool blend_points;
			
			//! whether to use the framework shader, set to true in constructor
			/*! If framework shader is not used, standard OpenGL 1.0 is used and \c use_point_size_array, 
			    \c backface_culling, \c outline_width_from_pixel, \c percentual_outline_width, \c orient_splats are ignored */
			bool use_point_shader;
			/// construct with default values
			point_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API point_renderer : public surface_renderer
		{
		protected:
			bool has_point_sizes;
			bool has_group_point_sizes;
			float reference_point_size;
			float y_view_angle;
			/// overload to allow instantiation of point_renderer
			render_style* create_render_style() const;
		public:
			///
			point_renderer();
			///
			bool init(cgv::render::context& ctx);
			///
			void set_reference_point_size(float _reference_point_size);
			///
			void set_y_view_angle(float y_view_angle);
			///
			template <typename T = float>
			void set_point_size_attribute(cgv::render::context& ctx, const std::vector<T>& point_sizes) { has_point_sizes = true; ref_prog().set_attribute_array(ctx, "point_size", point_sizes); }
			///
			template <typename T = float>
			void set_group_point_sizes(cgv::render::context& ctx, const std::vector<T>& group_point_sizes) { has_group_point_sizes = true; ref_prog().set_uniform(ctx, "group_point_sizes", group_point_sizes); }
			///
			bool validate_attributes(context& ctx);
			///
			bool enable(cgv::render::context& ctx);
			///
			bool disable(cgv::render::context& ctx);
		};
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			struct CGV_API point_render_style : public cgv::render::point_render_style
			{
				bool self_reflect(cgv::reflect::reflection_handler& rh);
			};
		}
		extern CGV_API cgv::reflect::extern_reflection_traits<cgv::render::point_render_style, cgv::reflect::render::point_render_style> get_reflection_traits(const cgv::render::point_render_style&);
	}
}

#include <cgv/config/lib_end.h>