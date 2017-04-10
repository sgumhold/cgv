#pragma once

#include "group_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		/** style of a point */
		struct CGV_API surface_render_style : public group_render_style
		{
			/// default value assigned to point color attribute in \c enable method of point renderer, set to (0,0,0,1) in constructor
			cgv::media::illum::phong_material::color_type surface_color;
			/// culling mode for point splats, set to CM_OFF in constructor
			CullingMode culling_mode;
			/// illumination mode defaults to \c IM_ONE_SIDED
			IlluminationMode illumination_mode;
			//! material side[s] where color is to be mapped to the diffuse material component, defaults to MS_FRONT_AND_BACK
			/*! */
			cgv::render::MaterialSide map_color_to_material;
			///
			cgv::media::illum::phong_material front_material;
			///
			cgv::media::illum::phong_material back_material;
			///
			surface_render_style();
		};

		/// base classes for renderers that support surface rendering
		class CGV_API surface_renderer : public group_renderer
		{
		protected:
			bool has_normals;
		public:
			surface_renderer();
			/// 
			bool enable(context& ctx);
			///
			bool disable(context& ctx);
			/// templated method to set the normal attribute from a vector of normals of type T, which should have 3 components
			template <typename T>
			void set_normal_array(context& ctx, const std::vector<T>& normals) { has_normals = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "normal"), normals); }
			/// templated method to set the normal attribute from an array of normals of type T, which should have 3 components
			template <typename T>
			void set_normal_array(context& ctx, const T* normals, size_t nr_elements, size_t stride_in_bytes = 0) { has_normals = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "normal"), normals, nr_elements, stride_in_bytes); }
		};
	}
}

namespace cgv {
	namespace reflect {
		namespace render {
			struct CGV_API surface_render_style : public cgv::render::surface_render_style
			{
				bool self_reflect(cgv::reflect::reflection_handler& rh);
			};
		}
		extern CGV_API cgv::reflect::extern_reflection_traits<cgv::render::surface_render_style, cgv::reflect::render::surface_render_style> get_reflection_traits(const cgv::render::surface_render_style&);
	}
}

#include <cgv/config/lib_end.h>