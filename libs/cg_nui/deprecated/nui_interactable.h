#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		enum ScalingMode
		{
			SM_NONE,
			SM_UNIFORM,
			SM_NON_UNIFORM
		};

		enum InteractionCapabilities
		{
			IC_NONE = 0,
			IC_INTERSECT = 1,
			IC_APPROACH = 2,
			IC_TRANSLATE_X = 4,
			IC_TRANSLATE_Y = 8,
			IC_TRANSLATE_Z = 16,
			IC_TRANSLATE = IC_TRANSLATE_X | IC_TRANSLATE_Y | IC_TRANSLATE_Z,
			IC_ROTATE_X = 32,
			IC_ROTATE_Y = 64,
			IC_ROTATE_Z = 128,
			IC_ROTATE = IC_ROTATE_X | IC_ROTATE_Y | IC_ROTATE_Z,
			IC_UNIFORM_SCALE = 256,
			IC_SCALE = 512,
			IC_THROW = 1024,
			IC_ALL = 2047
		};

		class CGV_API nui_interactable : public cgv::render::render_types
		{
		protected:
			/// whether to use scale also in node transformation
			ScalingMode scaling_mode;
			///
			InteractionCapabilities interaction_capabilities;
		public:
			nui_interactable(ScalingMode _scaling_mode = SM_NONE, InteractionCapabilities ic = IC_ALL);
			InteractionCapabilities get_interaction_capabilities() const { return interaction_capabilities; }
			void set_interaction_capabilities(InteractionCapabilities ic);
			bool intersectable() const { return (interaction_capabilities & IC_INTERSECT) != 0; }
			bool approachable() const { return (interaction_capabilities & IC_APPROACH) != 0; }
			bool translatable() const { return (interaction_capabilities & IC_TRANSLATE) != 0; }
			bool rotatable() const { return (interaction_capabilities & IC_ROTATE) != 0; }
			bool scalable() const { return (scaling_mode == SM_NON_UNIFORM) && ((interaction_capabilities & IC_SCALE) != 0); }
			bool uniform_scalable() const { return (scaling_mode == SM_UNIFORM) && ((interaction_capabilities & IC_UNIFORM_SCALE) != 0); }
			bool throwable() const { return (interaction_capabilities & IC_THROW) != 0; }

			ScalingMode get_scaling_mode() const { return scaling_mode; };
			void set_scaling_mode(ScalingMode _scaling_mode) { scaling_mode = _scaling_mode; };
		};
	}
}

#include <cgv/config/lib_end.h>