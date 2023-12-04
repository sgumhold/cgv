
#include "transformation_matrix_provider.h"
#include <cgv/math/inv.h>

namespace cgv {
	namespace nui {

mat4 transformation_matrix_provider::get_inverse_transformation_matrix() const {
	return inv(get_transformation_matrix()); 
}

	}
}

#include <cgv/config/lib_end.h>