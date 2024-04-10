#pragma once

#include "focusable.h"
#include <cgv/math/fvec.h>

namespace cgv {
	namespace nui {
		struct hit_info
		{
			vec3  hit_point;
			vec3  hit_normal;
			size_t primitive_index = 0;
		};

		struct hit_dispatch_info : public dispatch_info
		{
			virtual const hit_info* get_hit_info() const = 0;
		};
	}
}