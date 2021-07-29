#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		class CGV_API rounded_cone_renderer;

		//! reference to a singleton rounded cone renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API rounded_cone_renderer& ref_rounded_cone_renderer(context& ctx, int ref_count_change = 0);

		struct CGV_API rounded_cone_render_style : public surface_render_style
		{	
			/// multiplied to the sphere radii, initialized to 1
			float radius_scale;
			/// default value assigned to radius attribute in \c enable method of rounded cone renderer, set to 1 in constructor
			float radius;

			bool enable_texturing;
			enum TextureBlendMode {
				TBM_MIX = 0,
				TBM_TINT = 1,
				TBM_AVERAGE = 2,
				TBM_MULTIPLY = 3,
				TBM_INVERSE_MULTIPLY = 4,
				TBM_ADD = 5,
			} texture_blend_mode;
			float texture_blend_factor;
			bool texture_tile_from_center;
			vec2 texture_offset;
			vec2 texture_tiling;
			bool texture_use_reference_length;
			float texture_reference_length;

			bool enable_ambient_occlusion;
			float ao_offset;
			float ao_distance;
			float ao_strength;

			vec3 tex_offset;
			vec3 tex_scaling;
			vec3 tex_coord_scaling;
			float texel_size;
			float cone_angle_factor;
			std::vector<vec3> sample_dirs;
			/// construct with default values
			rounded_cone_render_style();
		};

		/// renderer that supports raycasting of rounded cones
		class CGV_API rounded_cone_renderer : public surface_renderer
		{
		protected:
			bool has_radii;
			/// the shader defines used to build the shader, used to comapre against new defines to determine if the shader needs to be rebuilt
			shader_define_map shader_defines;
			/// overload to allow instantiation of rounded_cone_renderer
			render_style* create_render_style() const;

			texture* albedo_texture;
			texture* density_texture;

		public:
			/// initializes member variables
			rounded_cone_renderer();
			/// call this before setting attribute arrays to manage attribute array in given manager
			void enable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// call this after last render/draw call to ensure that no other users of renderer change attribute arrays of given manager
			void disable_attribute_array_manager(const context& ctx, attribute_array_manager& aam);
			/// construct shader programs and return whether this was successful, call inside of init method of drawable
			bool init(context& ctx);

			bool set_albedo_texture(texture* tex);
			bool set_density_texture(texture* tex);

			///
			shader_define_map build_define_map();
			///
			bool build_shader(context& ctx, const shader_define_map& defines = shader_define_map());
			///
			bool enable(context& ctx);
			///
			template <typename T = float>
			void set_radius_array(const context& ctx, const std::vector<T>& radii) { has_radii = true; set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "radius"), radii); }
			/// 
			template <typename T = float>
			void set_radius_array(const context& ctx, const T* radii, size_t nr_elements, unsigned stride_in_bytes = 0) { has_radii = true;  set_attribute_array(ctx, ref_prog().get_attribute_location(ctx, "radius"), radii, nr_elements, stride_in_bytes); }
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool disable(context& ctx);
			/// convenience function to render with default settings
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};

		struct CGV_API rounded_cone_render_style_reflect : public rounded_cone_render_style
		{
			bool self_reflect(cgv::reflect::reflection_handler& rh);
		};
		extern CGV_API cgv::reflect::extern_reflection_traits<rounded_cone_render_style, rounded_cone_render_style_reflect> get_reflection_traits(const rounded_cone_render_style&);
	}
}

#include <cgv/config/lib_end.h>