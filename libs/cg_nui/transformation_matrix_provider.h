#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

using namespace render;

/// interface for classes that provide a transformation matrix
class CGV_API transformation_matrix_provider
{
	/// return transformation matrix
	virtual mat4 get_transformation_matrix() const = 0;
	/// return inverse of transformation matrix with default implementation via matrix inversion
	virtual mat4 get_inverse_transformation_matrix() const;
};

	}
}

#include <cgv/config/lib_end.h>