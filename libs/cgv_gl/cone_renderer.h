#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<

		class CGV_API cone_renderer;

		//! reference to a singleton cone renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API cone_renderer& ref_cone_renderer(context& ctx, int ref_count_change = 0);

		struct CGV_API cone_render_style : public surface_render_style {
			/// multiplied to the cone radii
			float radius_scale = 1.0f;
			/// default value assigned to radius attribute in \c enable method of cone renderer
			float radius = 1.0f;

			bool show_caps = true;
			bool rounded_caps = false;

			bool enable_texturing = false;
			enum TextureBlendMode {
				TBM_MIX = 0,
				TBM_TINT = 1,
				TBM_MULTIPLY = 2,
				TBM_INVERSE_MULTIPLY = 3,
				TBM_ADD = 4,
			} texture_blend_mode = TextureBlendMode::TBM_MIX;
			float texture_blend_factor = 1.0f;
			bool texture_tile_from_center = false;
			vec2 texture_offset = { 0.0f };
			vec2 texture_tiling = { 1.0f };
			bool texture_use_reference_length = false;
			float texture_reference_length = 1.0f;

			bool enable_ambient_occlusion = false;
			float ao_offset = 0.04f;
			float ao_distance = 0.8f;
			float ao_strength = 1.0f;

			vec3 tex_offset = { 0.0f };
			vec3 tex_scaling = { 1.0f };
			vec3 tex_coord_scaling = { 1.0f };
			float texel_size = 1.0f;
			float cone_angle_factor = 1.0f;
			std::vector<vec3> sample_dirs = std::vector<vec3>(3, { 0.0f, 1.0f, 0.0f });
		};

		/// renderer that supports raycasting of cones
		class CGV_API cone_renderer : public surface_renderer {
		protected:
			bool has_radii = false;
			/// return the default shader program name
			std::string get_default_prog_name() const override { return "cone.glpr"; }
			/// create and return the default render style
			render_style* create_render_style() const override { return new cone_render_style(); }
			/// update shader program compile options based on render style
			void update_shader_program_options(shader_compile_options& options) const override;

			texture* albedo_texture = nullptr;
			texture* density_texture = nullptr;

		public:
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			bool set_albedo_texture(texture* tex);
			bool set_density_texture(texture* tex);
			///
			bool enable(context& ctx);
			///
			template <typename T = float>
			void set_radius_array(const context& ctx, const std::vector<T>& radii) { has_radii = true; set_attribute_array(ctx, "radius", radii); }
			///
			template <typename T = float>
			void set_radius_array(const context& ctx, const T* radii, size_t nr_elements, unsigned stride_in_bytes = 0) { has_radii = true; set_attribute_array(ctx, "radius", radii, nr_elements, stride_in_bytes); }
			/// remove the radius attribute
			void remove_radius_array(const context& ctx);
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
			bool disable(context& ctx);
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
					  bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
			/// the clear function destructs the shader program and resets the texture pointers
			virtual void clear(const context& ctx);
		};

		struct CGV_API cone_render_style_reflect : public cone_render_style {
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};

		extern CGV_API cgv::reflect::extern_reflection_traits<cone_render_style, cone_render_style_reflect> get_reflection_traits(const cone_render_style&);
	}
}

#include <cgv/config/lib_end.h>