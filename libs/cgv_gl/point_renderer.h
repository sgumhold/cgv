#pragma once

#include "group_renderer.h"

#include "gl/lib_begin.h"

namespace cgv {
	namespace render {
		class CGV_API point_renderer;

		//! reference to a singleton point renderer that can be shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API point_renderer& ref_point_renderer(context& ctx, int ref_count_change = 0);

		/** style of a point */
		struct CGV_API point_render_style : public group_render_style
		{
			/*@name point rendering attributes*/
			//@{
			/// default value assigned to point size attribute in \c enable method of point renderer, set to 1 in constructor
			float point_size;
			/// whether to use the 
			bool use_group_point_size;
			/// whether to measure point size in pixels or in world space relative to reference_pixel_size passed to enable method, defaults to true
			bool measure_point_size_in_pixel;
			/// whether to span point splat in screen aligned coordinate system
			bool screen_aligned;
			//@}

			/*@name global point rendering options*/
			//@{
			/// default value for depth offset used to support layering
			float default_depth_offset;
			/// set to 1 in constructor 
			float blend_width_in_pixel;
			/// set to 0 in constructor
			float halo_width_in_pixel;
			/// set to 0 in constructor
			float percentual_halo_width;
			/// color of halo with opacity channel
			rgba halo_color;
			/// strength in [0,1] of halo color with respect to color of primitive
			float halo_color_strength;
			/// set to true in constructor
			bool blend_points;
			/// construct with default values
			point_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API point_renderer : public group_renderer
		{
		protected:
			bool has_point_sizes;
			bool has_group_point_sizes;
			bool has_indexed_colors;
			bool has_depth_offsets;
			float reference_point_size;
			float y_view_angle;
			/// overload to allow instantiation of point_renderer
			render_style* create_render_style() const;
			/// build point program
			bool build_shader_program(context& ctx, shader_program& prog, const shader_define_map& defines);
		public:
			///
			point_renderer();
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			///
			void set_reference_point_size(float _reference_point_size);
			///
			void set_y_view_angle(float y_view_angle);
			///
			template <typename T = float>
			void set_point_size_array(const context& ctx, const std::vector<T>& point_sizes) { has_point_sizes = true; set_attribute_array(ctx, "point_size", point_sizes); }
			/// set per point depth offsets
			template <typename T = float>
			void set_depth_offset_array(const context& ctx, const std::vector<T>& depth_offsets) { has_depth_offsets = true; set_attribute_array(ctx, "depth_offset", depth_offsets); }
			///
			template <typename T = unsigned, typename C = cgv::media::color<float,cgv::media::RGB,cgv::media::OPACITY> >
			void set_indexed_color_array(const context& ctx, const std::vector<T>& color_indices, const std::vector<C>& palette) {
				has_indexed_colors = true; 
				set_attribute_array(ctx, "color_index", color_indices); 
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
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};
		struct CGV_API point_render_style_reflect : public point_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<point_render_style, point_render_style_reflect> get_reflection_traits(const point_render_style&);
	}
}

#include <cgv/config/lib_end.h>