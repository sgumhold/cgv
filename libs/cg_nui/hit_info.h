#pragma once

#include "focusable.h"
#include <cgv/render/render_types.h>

namespace cgv {
	namespace nui {
		struct hit_info : public cgv::render::render_types
		{
			vec3  hit_point;
			vec3  hit_normal;
			size_t primitive_index = 0;
			vec3 hid_position;
			vec3 hid_direction;
		};

		struct hit_dispatch_info : public dispatch_info
		{
			virtual const hit_info* get_hit_info() const = 0;
		};
	}
}