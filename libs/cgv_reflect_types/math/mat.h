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
			rh.reflect_member("ncols", this->_ncols) &&
			rh.reflect_member("nrows", this->_nrows) &&
			rh.reflect_member("data", this->_data);
	}
};
		}
#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace math {
#endif
		template <typename T>
		cgv::reflect::extern_string_reflection_traits<cgv::math::mat<T>, cgv::reflect::math::mat<T> > get_reflection_traits(const cgv::math::mat<T>&) { return cgv::reflect::extern_string_reflection_traits<cgv::math::mat<T>, cgv::reflect::math::mat<T> >(); }

	}
}
