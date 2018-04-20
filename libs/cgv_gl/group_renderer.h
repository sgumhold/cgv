#pragma once

#include "renderer.h"
#include <cgv/reflect/reflect_extern.h>
#include <cgv/render/shader_program.h>
#include <cgv_reflect_types/render/context.h>
#include <cgv_reflect_types/media/illum/phong_material.h>
#include <cgv/media/illum/phong_material.hh>

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		/** render style used for group information */
		struct CGV_API group_render_style : public render_style
		{
			/// whether to use group colors indexed through group index, defaults to false
			bool use_group_color;
			/// whether to use group translation and rotation indexed through group index, defaults to false
			bool use_group_transformation;
			/// set default values
			group_render_style();
		};

		/// abstract renderer class that provides functionality for grouping primitives
		class CGV_API group_renderer : public renderer
		{
		protected:
			bool has_group_indices;
			bool has_group_colors;
			bool has_group_translations;
			bool has_group_rotations;
		public:
			///
			group_renderer();
			/// check additionally the group attributes
			bool validate_attributes(context& ctx) const;
			/// overload to activate group style
			bool enable(context& ctx);
			///
			bool disable(cgv::render::context& ctx);
			/// method to set the group index attribute
			void set_group_index_attribute(cgv::render::context& ctx, const std::vector<unsigned>& group_indices);
			/// method to set the group index attribute
			void set_group_index_attribute(cgv::render::context& ctx, const unsigned* group_indices, size_t nr_elements);
			/// template method to set the group colors from a vector of colors of type T
			template <typename T>
			void set_group_colors(cgv::render::context& ctx, const std::vector<T>& colors) { has_group_colors = true;  ref_prog().set_uniform_array(ctx, "group_colors", colors); }
			/// template method to set the group colors from a vector of colors of type T
			template <typename T>
			void set_group_colors(cgv::render::context& ctx, const T* colors, size_t nr_elements) { has_group_colors = true;  ref_prog().set_uniform_array(ctx, "group_colors", colors, nr_elements); }
			/// template method to set the group translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_group_translations(cgv::render::context& ctx, const std::vector<T>& group_translations) { has_group_translations = true; ref_prog().set_uniform_array(ctx, "group_translations", group_translations); }
			/// template method to set the group translations from a vector of vectors of type T, which should have 3 components
			template <typename T>
			void set_group_translations(cgv::render::context& ctx, const T* group_translations, size_t nr_elements) { has_group_translations = true; ref_prog().set_uniform_array(ctx, "group_translations", group_translations, nr_elements); }
			/// template method to set the group rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_group_rotations(cgv::render::context& ctx, const std::vector<T>& group_rotations) { has_group_rotations = true; ref_prog().set_uniform_array(ctx, "group_rotations", group_rotations); }
			/// template method to set the group rotation from a vector of quaternions of type T, which should have 4 components
			template <typename T>
			void set_group_rotations(cgv::render::context& ctx, const T* group_rotations, size_t nr_elements) { has_group_rotations = true; ref_prog().set_uniform_array(ctx, "group_rotations", group_rotations, nr_elements); }
		};
	}
}


namespace cgv {
	namespace reflect {
		namespace render {
			struct CGV_API group_render_style : public cgv::render::group_render_style
			{
				bool self_reflect(cgv::reflect::reflection_handler& rh);
			};
		}
		extern CGV_API cgv::reflect::extern_reflection_traits<cgv::render::group_render_style, cgv::reflect::render::group_render_style> get_reflection_traits(const cgv::render::group_render_style&);
	}
}


#include <cgv/config/lib_end.h>