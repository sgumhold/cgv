#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		class primitive_container;

		struct contact_info : public render::render_types
		{
			struct contact
			{
				float distance;
				vec3 position;
				vec3 normal;
				vec3 texcoord; // used to store 2D or 3D local coordinates
				uint32_t primitive_index;
				primitive_container* container;
			};

			std::vector<contact> contacts;
		};
	}
}

#include <cgv/config/lib_end.h>