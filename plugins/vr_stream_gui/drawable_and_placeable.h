#pragma once

#include <cgv/render/drawable.h>
#include "placeable.h"

namespace cgv {
namespace render {
	// simple wrapper for drawable and placable in one
	class drawable_and_placeable : public drawable, public placeable {
	};

} // namespace render
} // namespace cgv
