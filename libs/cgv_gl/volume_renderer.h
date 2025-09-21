#pragma once

#include "renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		
		class CGV_API volume_renderer;

		//! reference to a singleton volume renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singleton renderer is destructed. */
		extern CGV_API volume_renderer& ref_volume_renderer(context& ctx, int ref_count_change = 0);

		/** style of a volume */
		struct CGV_API volume_render_style : public render_style {
			/// quality measure for the number of steps used during ray marching
			enum IntegrationQuality {
				IQ_8 = 8,
				IQ_16 = 16,
				IQ_32 = 32,
				IQ_64 = 64,
				IQ_128 = 128,
				IQ_256 = 256,
				IQ_512 = 512,
				IQ_1024 = 1024,
				IQ_2048 = 2048,
				IQ_4096 = 4096
			} integration_quality = IQ_128;
			/// whether to use the noise texture to offset ray start positions in order to reduce sampling artifacts
			bool enable_noise_offset = true;
			/// the interpolation method used (supplied volume texture should be set to GL_LINEAR)
			enum InterpolationMode {
				IP_NEAREST = 0,		/// only the closest voxel is sampled
				IP_SMOOTHED = 1,	/// modification of the built-in trilinear interpolation to prevent triangular artifacts (results look blockier in the volume, mid way between nearest and linear)
				IP_LINEAR = 2,		/// default built-in trilinear interpolation
				IP_CUBIC = 3		/// tricubic interpolation using 8 modified trlinear samples
			} interpolation_mode = IP_LINEAR;
			/// whether to enable depth testing by reading depth from a texture to allow geometry intersecting the volume (depth texture must be supplied)
			bool enable_depth_test = true;
			// the opacity threshold needed to pass before the volume is considered a solid surface (needed for defining depth and supporting focus picking)
			float picking_opacity_threshold = 0.03f;

			/// the compositing mode used
			enum CompositingMode {
				CM_MAXIMUM_INTENSITY_PROJECTION = 0,
				CM_AVERAGE = 1,
				CM_BLEND = 2 // using transfer function
			} compositing_mode = CM_BLEND;
			
			/// the coefficient used to adjust sample opacity based on volume scaling (useful range between 50 and 500)
			float scale_adjustment_factor = 100.0f;
			
			/// whether to enable lighting
			bool enable_lighting = false;
			/// whether the light is local to the eye position (moves with the eye) or is static to the scene
			bool light_local_to_eye = true;
			/// whether to use a supplied gradient texture or compute gradients on the fly via central differences (default)
			bool use_gradient_texture = false;
			/// the direction of the directional light
			vec3 light_direction = normalize(cgv::vec3(-1.0f, 1.0f, 1.0f));
			/// light ambient component strength
			float ambient_strength = 0.3f;
			/// material diffuse component strength
			float diffuse_strength = 0.8f;
			/// material specular component strength
			float specular_strength = 0.4f;
			/// material roughness (inversely proportional to specular shininess)
			float roughness = 0.3f;
			/// material specular color mix factor (0 = color from transfer function, 1 = pure white)
			float specular_color_mix = 0.0f;

			/// whether to enable modulating the volume opacity by the gradient magnitude
			bool enable_gradient_modulation = false;
			///  influence scale for gradient-based opacity modulation
			float gradient_lambda = 0.0f;

			/// mode of a single supported isosurface
			enum IsosurfaceMode{
				IM_NONE = 0,			/// not enabled
				IM_ISOVALUE = 1,		/// based on volume value (volume >= isovalue)
				IM_ALPHA_THRESHOLD = 2	/// based on opacity value from transfer function (tf(volume).a >= isovalue)
			} isosurface_mode = IM_NONE;
			/// the value used to check for an isosurface
			float isovalue = 0.5f;
			/// the default constant isosurface color
			rgb isosurface_color = { 0.7f };
			/// whether to color the isosurface based on the transfer function
			bool isosurface_color_from_transfer_function = false;

			/// mode of slice rendering
			enum SliceMode {
				SM_DISABLED = 0,   // no slice
				SM_OPAQUE = 1,     // opaque slice rendering
				SM_TRANSPARENT = 2 // transparent slice rendering 
			} slice_mode = SM_DISABLED;
			/// coordinate axis orthogonal to which slice is rendered
			int slice_axis = 2;
			/// coordinate value along axis defining slice in range [0,1]
			float slice_coordinate = 0.5f;
			/// in case of transparent mode, slice opacity
			float slice_opacity = 0.5f;

			/// a bounding box used to define a subspace of the volume to be visualized
			box3 clip_box = { { 0.0f }, { 1.0f } };
		};

		/// renderer that supports point splatting
		class CGV_API volume_renderer : public renderer
		{
		private:
			/// a private attribute array manager that holds position data that is constant for all volumes
			cgv::render::attribute_array_manager position_aam;
		protected:
			/// the 3D texture used for rendering
			texture* volume_texture = nullptr;
			/// the 2D transfer function texture used for classification of the volume values
			texture* transfer_function_texture = nullptr;
			/// a 2D texture containing random noise used to offset ray start positions in order to reduce ring artifacts
			texture noise_texture = texture("uint8[R]", TF_LINEAR, TF_LINEAR, TW_REPEAT, TW_REPEAT);
			/// the 3D texture containing vector gradients used for lighting normal calculation
			texture* gradient_texture = nullptr;
			/// a 2D texture from a frame buffer depth buffer used to combine volume rendering with opaque geometry
			texture* depth_texture = nullptr;
			/// the bounding box of the volume in scene units
			box3 bounding_box = { { 0.0f }, { 1.0f } };
			/// whether to translate and scale the volume to the given bounding box during rendering
			bool apply_bounding_box_transformation = false;
			/// offset applied to the noise texture (can be used in conjunction with temporal anti aliasing)
			vec2 noise_offset = { 0.0f };

			/// return the default shader program name
			std::string get_default_prog_name() const override { return "volume.glpr"; }
			/// create and return the default render style
			render_style* create_render_style() const override { return new volume_render_style(); }
			/// update shader program compile options based on render style
			void update_shader_program_options(shader_compile_options& options) const override;
			/// initializes the noise texture with random values
			void init_noise_texture(context& ctx);
		public:
			/// construct shader programs and return whether this was successful, call inside of init method of drawable
			bool init(context& ctx);
			/// clean up
			virtual void clear (const context& ctx);
			/// sets the 3D volume texture containing scalar values (density or other measured quantities)
			bool set_volume_texture(texture* tex);
			/// sets the transfer function used for classification; must be 1D or 2D (as loaded from an image)
			bool set_transfer_function_texture(texture* tex);
			/// sets the gradient texture used for lighting
			bool set_gradient_texture(texture* tex);
			/// sets the depth texture needed for rendering with additional opaque geometry
			bool set_depth_texture(texture* tex);
			///
			void set_bounding_box(const box3& bbox);
			///
			void transform_to_bounding_box(bool flag);
			///
			void set_noise_offset(const vec2& offset);
			///
			bool enable(context& ctx);
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool disable(context& ctx);
			///
			void draw(context& ctx, size_t start, size_t count,
				bool use_strips = false, bool use_adjacency = false, uint32_t strip_restart_index = -1);
		};
	}
}

#include <cgv/config/lib_end.h>