#pragma once

#include "line_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		class CGV_API normal_renderer;

		//! reference to a singleton normal renderer that can be shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singelton renderer is destructed. */
		extern CGV_API normal_renderer& ref_normal_renderer(context& ctx, int ref_count_change = 0);


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
			bool validate_attributes(const context& ctx) const;
		public:
			normal_renderer();
			void set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr);
			/// the normal scale is multiplied to the normal length of the normal render style
			void set_normal_scale(float _normal_scale);
			bool init(context& ctx);
			bool enable(context& ctx);
			/// templated method to set the normal attribute from a vector of normals of type T, which should have 3 components
			template <typename T>
			void set_normal_array(const context& ctx, const std::vector<T>& normals) { has_normals = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "normal"), normals); }
			/// templated method to set the normal attribute from an array of normals of type T, which should have 3 components
			template <typename T>
			void set_normal_array(const context& ctx, const T* normals, size_t nr_elements, unsigned stride_in_bytes = 0) { has_normals = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "normal"), normals, nr_elements, stride_in_bytes); }
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};

		struct CGV_API normal_render_style_reflect : public normal_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<normal_render_style, normal_render_style_reflect> get_reflection_traits(const cgv::render::normal_render_style&);
	}
}

#include <cgv/config/lib_end.h>