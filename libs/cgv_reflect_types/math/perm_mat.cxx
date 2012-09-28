#include "perm_mat.h"

namespace cgv {
	namespace reflect {
		namespace math {

bool perm_mat::self_reflect(cgv::reflect::reflection_handler& rh) 
{
		return rh.reflect_member("data", _data);
}

		}

extern_reflection_traits<cgv::math::perm_mat, cgv::reflect::math::perm_mat> get_reflection_traits(const cgv::math::perm_mat&) 
{
	return extern_reflection_traits<cgv::math::perm_mat, cgv::reflect::math::perm_mat>(); 
}

	}
}
