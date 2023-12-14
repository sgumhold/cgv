#pragma once

#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

using namespace render;

/// interface for classes that provide a transformation matrix
class CGV_API transformation_matrix_provider
{
public:
	/// check whether there is a transformation applied before the one provided here (defaults to false)
	virtual bool has_pre_transformation() const;
	/// in case there is a pre transformation return homogeneous transformation matrix (defaults to identity matrix)
	virtual mat4 get_pre_transformation_matrix() const;
	/// in case there is a pre transformation return homogeneous inverse transformation matrix (defaults to identity matrix)
	virtual mat4 get_inverse_pre_transformation_matrix() const;
	/// check whether there is a transformation applied after the one provided here (defaults to false)
	virtual bool has_post_transformation() const;
	/// in case there is a post transformation return homogeneous transformation matrix (defaults to identity matrix)
	virtual mat4 get_post_transformation_matrix() const;
	/// in case there is a post transformation return homogeneous inverse transformation matrix (defaults to identity matrix)
	virtual mat4 get_inverse_post_transformation_matrix() const;
	/// return partial transformation matrix up to the one resulting from current transformation matrix provider
	virtual mat4 get_partial_transformation_matrix() const;
	/// return inverse of partial transformation matrix up to the one resulting from current transformation matrix provider
	virtual mat4 get_inverse_partial_transformation_matrix() const;
	/// return transformation matrix
	virtual mat4 get_transformation_matrix() const = 0;
	/// return inverse of transformation matrix with default implementation via matrix inversion
	virtual mat4 get_inverse_transformation_matrix() const;
};

	}
}

#include <cgv/config/lib_end.h>