#pragma once

#include <cgv_reflect_types/math/fvec.h>
#include <cgv/math/quaternion.h>
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

/**
 * An axis aligned box, defined by to points: min and max
 */
template<typename T>
struct quaternion : public cgv::math::quaternion<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return rh.reflect_base(static_cast<cgv::math::fvec<T,4>&>(*this));
	}
};

		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace math {
#endif

template<typename T>
cgv::reflect::extern_reflection_traits<cgv::math::quaternion<T>, cgv::reflect::math::quaternion<T> > get_reflection_traits(const cgv::math::quaternion<T>&) { return cgv::reflect::extern_reflection_traits<cgv::math::quaternion<T>, cgv::reflect::math::quaternion<T> >(); }

	}
}

