#pragma once

#include <cgv/math/vec.h>
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T>
struct vec : public cgv::math::vec<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return 
			rh.reflect_member("data_is_external", data_is_external) &&
			rh.reflect_array("coords", _data, _size);
	}
};
		}
template <typename T>
extern_string_reflection_traits<cgv::math::vec<T>, cgv::reflect::math::vec<T> > get_reflection_traits(const cgv::math::vec<T>&) { return extern_string_reflection_traits<cgv::math::vec<T>, cgv::reflect::math::vec<T> >(); }

	}
}
