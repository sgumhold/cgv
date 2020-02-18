#pragma once

#include "surface_renderer.h"

#include "gl/lib_begin.h"

namespace cgv { // @<
	namespace render { // @<
		
		/// volume texture can have different interpolation methods
		enum InterpolationMode {
			IP_NEAREST = 0,
			IP_LINEAR = 1,
			IP_SMOOTH = 2,
			IP_CUBIC = 3
		};

		class CGV_API volume_renderer;

		//! reference to a singleton volume renderer that is shared among drawables
		/*! the second parameter is used for reference counting. Use +1 in your init method,
			-1 in your clear method and default 0 argument otherwise. If internal reference
			counter decreases to 0, singelton renderer is destructed. */
		extern CGV_API volume_renderer& ref_volume_renderer(context& ctx, int ref_count_change = 0);

		/** style of a volume */
		struct CGV_API volume_render_style : public render_style {
			/*@name global volume rendering options*/
			//@{
			///
			int volume_texture_unit;
			int transfer_function_texture_unit;
			float alpha;
			float level_of_detail;
			float step_size;
			vec3 tex_size;
			vec3 tex_coord_scaling;
			mat4 transformation;

			/// the interpolation method used
			InterpolationMode interpolation_mode;
			/// construct with default values
			volume_render_style();
		};

		/// renderer that supports point splatting
		class CGV_API volume_renderer : public renderer
		{
		protected:
			///
			cgv::render::attribute_array_manager aa_manager;
			/// the eye position in world space
			vec3 eye_position;
			/// whether the shader should be rebuilt after a define update
			std::string shader_defines;
			/// store whether attr array is specified
			bool has_attr;
			/// overload to allow instantiation of volume_renderer
			render_style* create_render_style() const;
		public:
			/// initializes position_is_center to true 
			volume_renderer();
			///
			void set_attribute_array_manager(const context& ctx, attribute_array_manager* _aam_ptr);
			/// construct shader programs and return whether this was successful, call inside of init method of drawable
			bool init(context& ctx);
			///
			void set_eye_position(vec3 _eye_position);
			///
			std::string build_define_string();
			///
			bool build_shader(context& ctx, std::string defines = "");
			///
			bool enable(context& ctx);
			///
			bool validate_attributes(const context& ctx) const;
			///
			bool disable(context& ctx);
			///
			void draw(context& ctx, int offset, int count);
		};
	}
}

#include <cgv/config/lib_end.h>