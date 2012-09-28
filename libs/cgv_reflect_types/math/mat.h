#pragma once

#include <cgv/math/mat.h>
#include "vec.h"
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T>
struct mat : public cgv::math::mat<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return 
			rh.reflect_member("ncols", _ncols) &&
			rh.reflect_member("nrows", _nrows) &&
			rh.reflect_member("data", _data);
	}
};
		}

template <typename T>
extern_string_reflection_traits<cgv::math::mat<T>, cgv::reflect::math::mat<T> > get_reflection_traits(const cgv::math::mat<T>&) { return extern_string_reflection_traits<cgv::math::mat<T>, cgv::reflect::math::mat<T> >(); }

	}
}
