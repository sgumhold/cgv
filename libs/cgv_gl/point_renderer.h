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
			float point_size = 1.0f;
			/// whether to use the 
			bool use_group_point_size = false;
			/// whether to measure point size in pixels or in world space relative to reference_pixel_size passed to enable method, defaults to true
			bool measure_point_size_in_pixel = true;
			/// whether to span point splat in screen aligned coordinate system
			bool screen_aligned = true;
			//@}

			/*@name global point rendering options*/
			//@{
			/// default value for depth offset used to support layering
			float default_depth_offset = 0.0f;
			/// set to 1 in constructor 
			float blend_width_in_pixel = 1.0f;
			/// set to 0 in constructor
			float halo_width_in_pixel = 0.0f;
			/// set to 0 in constructor
			float percentual_halo_width = 0.0f;
			/// color of halo with opacity channel
			rgba halo_color = { 1.0f };
			/// strength in [0,1] of halo color with respect to color of primitive
			float halo_color_strength = 0.5f;
			/// whether to enable blending while rendering (needed for smooth edges; true by default)
			bool blend_points = true;
		};

		/// renderer that supports point splatting
		class CGV_API point_renderer : public group_renderer
		{
		protected:
			bool has_point_sizes = false;
			bool has_group_point_sizes = false;
			bool has_indexed_colors = false;
			bool has_depth_offsets = false;
			float reference_point_size = 0.01f;
			float y_view_angle = 45.0f;
			/// return the default shader program name
			std::string get_default_prog_name() const override { return "point.glpr"; }
			/// create and return the default render style
			render_style* create_render_style() const override { return new point_render_style(); }
		public:
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam) override;
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam) override;
			///
			void set_reference_point_size(float _reference_point_size);
			///
			void set_y_view_angle(float y_view_angle);
			///
			template <typename T = float>
			void set_point_size_array(const context& ctx, const std::vector<T>& point_sizes) { has_point_sizes = true; set_attribute_array(ctx, "point_size", point_sizes); }
			/// remove the point size attribute
			void remove_point_size_array(const context& ctx);
			/// set per point depth offsets
			template <typename T = float>
			void set_depth_offset_array(const context& ctx, const std::vector<T>& depth_offsets) { has_depth_offsets = true; set_attribute_array(ctx, "depth_offset", depth_offsets); }
			/// remove the depth offset attribute
			void remove_depth_offset_array(const context& ctx);
			///
			template <typename T = unsigned, typename C = cgv::media::color<float,cgv::media::RGB,cgv::media::OPACITY> >
			void set_indexed_color_array(const context& ctx, const std::vector<T>& color_indices, const std::vector<C>& palette) {
				has_indexed_colors = true; 
				set_attribute_array(ctx, "color_index", color_indices); 
				ref_prog().set_uniform_array(ctx, "palette", palette); 
			}
			/// remove the indexed color attribute
			void remove_indexed_color_array(const context& ctx);
			///
			template <typename T = float>
			void set_group_point_sizes(const context& ctx, const std::vector<T>& group_point_sizes) { has_group_point_sizes = true; ref_prog().set_uniform_array(ctx, "group_point_sizes", group_point_sizes); }
			///
			bool validate_attributes(const context& ctx) const override;
			///
			bool enable(context& ctx) override;
			///
			bool disable(context& ctx) override;
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1) override;
		};
		struct CGV_API point_render_style_reflect : public point_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<point_render_style, point_render_style_reflect> get_reflection_traits(const point_render_style&);
	}
}

#include <cgv/config/lib_end.h>