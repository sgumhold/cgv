#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		class CGV_API ellipsoid_renderer;

		//! reference to a singleton ellipsoid renderer that can be shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API ellipsoid_renderer& ref_ellipsoid_renderer(context& ctx, int ref_count_change = 0);

		/** render style for ellipsoid rendere */
		struct CGV_API ellipsoid_render_style : public surface_render_style
		{
			/*@name ellipsoid rendering attributes*/
			//@{
			/// multiplied to the ellipsoid sizes, initialized to 1
			float size_scale;
			/// default value assigned to size attribute in \c enable method of ellipsoid renderer, set to 1 in constructor
			cgv::vec3 size;
			//@}

			/// construct with default values
			ellipsoid_render_style();
		};

		/// renderer that supports splatting of ellipsoids
		class CGV_API ellipsoid_renderer : public surface_renderer
		{
		protected:
			bool has_sizes;
			bool has_orientations;
			/// overload to allow instantiation of point_renderer
			render_style* create_render_style() const;
			/// build ellipsoid program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
		public:
			///
			ellipsoid_renderer();
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			///
			template <typename T = float>
			void set_size(const context& ctx, const T& size) {
				has_sizes = true;
				ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "size"), size);
			}
			///
			template <typename T = float>
			void set_size_array(const context& ctx, const std::vector<T>& sizes) {
				has_sizes = true;
				set_attribute_array(ctx, "size", sizes);
			}
			/// 
			template <typename T>
			void set_size_array(const context& ctx, const T* sizes, size_t nr_elements, unsigned stride_in_bytes = 0) {
				has_sizes = true;
				set_attribute_array(ctx, "size", sizes, nr_elements, stride_in_bytes);
			}
			/// remove the size attribute
			void remove_size_array(const context& ctx);
			///
			template <typename T = float>
			void set_orientation(const context& ctx, const cgv::math::quaternion<T>& orientation) {
				has_orientations = true;
				ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "orientation"), orientation);
			}
			/// use this function if you store spheres in vec4 with the 4th component the radius
			template <typename T = float>
			void set_orientation_array(const context& ctx, const std::vector<cgv::math::quaternion<T>>& orientations) {
				has_orientations = true;
				set_attribute_array(ctx, "orientation", orientations);
			}
			/// 
			template <typename T>
			void set_orientation_array(const context& ctx, const T* orientations, size_t nr_elements, unsigned stride_in_bytes = 0) {
				has_orientations = true;
				set_attribute_array(ctx, "orientation", orientations, nr_elements, stride_in_bytes);
			}
			/// remove the orientation attribute
			void remove_orientation_array(const context& ctx);
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool enable(context& ctx);
			///
			bool disable(context& ctx);
			///
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};

		struct CGV_API ellipsoid_render_style_reflect : public ellipsoid_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<ellipsoid_render_style, ellipsoid_render_style_reflect> get_reflection_traits(const ellipsoid_render_style&);
	}
}


#include <cgv/config/lib_end.h>