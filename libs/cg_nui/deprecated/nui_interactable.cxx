#include <cgv/math/ftransform.h>
#include "nui_interactable.h"

namespace cgv {
	namespace nui {
		nui_interactable::nui_interactable(ScalingMode _scaling_mode, InteractionCapabilities ic) : scaling_mode(_scaling_mode)
		{
			interaction_capabilities = ic;
		}

		void nui_interactable::set_interaction_capabilities(InteractionCapabilities ic)
		{
			interaction_capabilities = ic;
		}

	}
}