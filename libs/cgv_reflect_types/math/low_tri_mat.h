#pragma once

#include <cgv/math/low_tri_mat.h>
#include "vec.h"
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T>
struct low_tri_mat : public cgv::math::low_tri_mat<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return 
			rh.reflect_member("dim", this->_dim) &&
			rh.reflect_member("data", this->_data);
	}
};
		}
#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace math {
#endif
		template <typename T>
		cgv::reflect::extern_reflection_traits<cgv::math::low_tri_mat<T>, cgv::reflect::math::low_tri_mat<T> >
			get_reflection_traits(const cgv::math::low_tri_mat<T>&) {
				return cgv::reflect::extern_reflection_traits<cgv::math::low_tri_mat<T>, cgv::reflect::math::low_tri_mat<T> >();
		}

	}
}

