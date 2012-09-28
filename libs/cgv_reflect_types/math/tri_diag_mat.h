#pragma once

#include <cgv/math/tri_diag_mat.h>
#include "vec.h"
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T>
struct tri_diag_mat : public cgv::math::tri_diag_mat<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return 
			rh.reflect_member("dim", _dim) &&
			rh.reflect_member("data", _data);
	}
};
		}

template <typename T>
extern_reflection_traits<cgv::math::tri_diag_mat<T>, cgv::reflect::math::tri_diag_mat<T> > get_reflection_traits(const cgv::math::tri_diag_mat<T>&) { return extern_reflection_traits<cgv::math::tri_diag_mat<T>, cgv::reflect::math::tri_diag_mat<T> >(); }

	}
}

