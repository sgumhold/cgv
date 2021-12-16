#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		class CGV_API sphere_renderer;

		//! reference to a singleton sphere renderer that can be shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API sphere_renderer& ref_sphere_renderer(context& ctx, int ref_count_change = 0);

		/** render style for sphere rendere */
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

		/// renderer that supports splatting of spheres
		class CGV_API sphere_renderer : public surface_renderer
		{
		protected:
			bool has_radii;
			bool has_group_radii;
			float y_view_angle;
			/// overload to allow instantiation of point_renderer
			render_style* create_render_style() const;
			/// build sphere program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
		public:
			///
			sphere_renderer();
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			///
			void set_y_view_angle(float y_view_angle);
			///
			template <typename T = float>
			void set_radius(const context& ctx, const T& radius) { has_radii = true; ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "radius"), radius); }
			///
			template <typename T = float>
			void set_radius_array(const context& ctx, const std::vector<T>& radii) { has_radii = true; set_attribute_array(ctx, "radius", radii); }
			/// 
			template <typename T = float>
			void set_radius_array(const context& ctx, const T* radii, size_t nr_elements, unsigned stride_in_bytes = 0) { has_radii = true;  set_attribute_array(ctx, "radius", radii, nr_elements, stride_in_bytes); }
			///
			template <typename T = float>
			void set_group_radii(const context& ctx, const std::vector<T>& group_radii) { has_group_radii = true; ref_prog().set_uniform_array(ctx, "group_radii", group_radii); }
			/// 
			template <typename T = float>
			void set_sphere(const context& ctx, const cgv::math::fvec<T, 4>& sphere) {
				ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "position"), (const cgv::math::fvec<T, 3>&)sphere);
				ref_prog().set_attribute(ctx, ref_prog().get_attribute_location(ctx, "radius"), sphere[3]);
				has_positions = true;
				has_radii = true;
			}
			/// use this function if you store spheres in vec4 with the 4th component the radius
			template <typename T = float>
			void set_sphere_array(const context& ctx, const std::vector<cgv::math::fvec<T, 4> >& spheres) {
				set_composed_attribute_array(ctx, "position", &spheres.front(), spheres.size(), reinterpret_cast<const cgv::math::fvec<T, 3>&>(spheres.front()));
				ref_composed_attribute_array(ctx, "radius", "position", &spheres.front(), spheres.size(), spheres[0][3]);
				has_positions = true;
				has_radii = true;
			}
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

		struct CGV_API sphere_render_style_reflect : public sphere_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<sphere_render_style, sphere_render_style_reflect> get_reflection_traits(const sphere_render_style&);
	}
}


#include <cgv/config/lib_end.h>