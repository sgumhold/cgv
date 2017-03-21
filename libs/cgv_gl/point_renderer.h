#pragma once

#include <cgv/reflect/reflection_handler.h>
#include <cgv/reflect/reflect_extern.h>
#include <cgv/reflect/reflect_enum.h>
#include <cgv/render/context.h>
#include <cgv/render/shader_program.h>
#include <cgv/media/illum/phong_material.hh>

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		
		enum IlluminationMode {
			IM_OFF, IM_ONE_SIDED, IM_TWO_SIDED
		};
		enum CullingMode {
			CM_OFF, CM_BACKFACE, CM_FRONTFACE
		};

		/** style of a point */
		struct CGV_API point_render_style
		{
			/*@name point rendering attributes*/
			//@{
			/// default value assigned to point size attribute in \c enable method of point renderer, set to 1 in constructor
			float point_size;
			/// default value assigned to point color attribute in \c enable method of point renderer, set to (0,0,0,1) in constructor
			cgv::media::illum::phong_material::color_type point_color;
			//@}

			/*@name global point rendering options*/
			//@{
			/// culling mode for point splats, set to CM_OFF in constructor
			CullingMode culling_mode;
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
			
			/// illumination mode defaults to \c IM_ONE_SIDED
			IlluminationMode illumination_mode;
			//! material side[s] where color is to be mapped to the diffuse material component, defaults to MS_FRONT_AND_BACK
			/*! */
			cgv::render::MaterialSide map_color_to_material;
			///
			cgv::media::illum::phong_material front_material;
			///
			cgv::media::illum::phong_material back_material;

			//! whether to use the framework shader, set to true in constructor
			/*! If framework shader is not used, standard OpenGL 1.0 is used and \c use_point_size_array, 
			    \c backface_culling, \c outline_width_from_pixel, \c percentual_outline_width, \c orient_splats are ignored */
			bool use_point_shader;
			/// construct with default values
			point_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API point_renderer
		{
		protected:
			cgv::render::shader_program point_prog;
		public:
			point_renderer();
			bool init(cgv::render::context& ctx);
			void enable(cgv::render::context& ctx, const point_render_style& prs, float reference_point_size, float y_view_angle, bool has_normals, bool has_colors, bool use_group_point_size = false, bool use_group_color = false);
			void disable(cgv::render::context& ctx, const point_render_style& prs, bool has_normals);
			void clear(cgv::render::context& ctx);
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
		extern CGV_API cgv::reflect::enum_reflection_traits<cgv::render::IlluminationMode> get_reflection_traits(const cgv::render::IlluminationMode&);
		extern CGV_API cgv::reflect::enum_reflection_traits<cgv::render::CullingMode> get_reflection_traits(const cgv::render::CullingMode&);
		extern CGV_API cgv::reflect::enum_reflection_traits<cgv::render::MaterialSide> get_reflection_traits(const cgv::render::MaterialSide&);
	}
}

#include <cgv/config/lib_end.h>