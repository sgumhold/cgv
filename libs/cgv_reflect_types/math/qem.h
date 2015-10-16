#pragma once

#include <cgv/math/qem.h>
#include "vec.h"
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T>
struct qem : public cgv::math::qem<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return rh.reflect_base(static_cast<cgv::math::vec<T>&>(*this));
	}
};

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace math {
#endif

template <typename T>
cgv::reflect::extern_string_reflection_traits<cgv::math::qem<T>, cgv::reflect::math::qem<T> > get_reflection_traits(const cgv::math::qem<T>&) { return cgv::reflect::extern_string_reflection_traits<cgv::math::qem<T>, cgv::reflect::math::qem<T> >(); }

	}
}
