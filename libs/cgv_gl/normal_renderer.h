#pragma once

#include "line_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {


		struct CGV_API normal_render_style : public line_render_style
		{
			float normal_length;
			normal_render_style();
		};

		/// renderer that supports rendering point normals
		class CGV_API normal_renderer : public line_renderer
		{
		protected:
			bool has_normals;
			/// scaling of normal length
			float normal_scale;
			/// overload to allow instantiation of box_wire_renderer
			render_style* create_render_style() const;
			bool validate_attributes(context& ctx);
		public:
			normal_renderer();
			/// the normal scale is multiplied to the normal length of the normal render style
			void set_normal_scale(float _normal_scale);
			bool init(context& ctx);
			bool enable(context& ctx);
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
			struct CGV_API normal_render_style : public cgv::render::normal_render_style
			{
				bool self_reflect(cgv::reflect::reflection_handler& rh);
			};
		}
		extern CGV_API cgv::reflect::extern_reflection_traits<cgv::render::normal_render_style, cgv::reflect::render::normal_render_style> get_reflection_traits(const cgv::render::normal_render_style&);
	}
}

#include <cgv/config/lib_end.h>