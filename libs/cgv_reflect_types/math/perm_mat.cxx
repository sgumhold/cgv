#include "perm_mat.h"

namespace cgv {
	namespace reflect {
		namespace math {

bool perm_mat::self_reflect(cgv::reflect::reflection_handler& rh) 
{
	cgv::math::vec<float> v;
	rh.reflect_member("v", v);
	return rh.reflect_member("data", this->_data);
}

		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv {	namespace math {
#endif
cgv::reflect::extern_reflection_traits<cgv::math::perm_mat, cgv::reflect::math::perm_mat> get_reflection_traits(const cgv::math::perm_mat&) 
{
	return cgv::reflect::extern_reflection_traits<cgv::math::perm_mat, cgv::reflect::math::perm_mat>(); 
}

	}
}
