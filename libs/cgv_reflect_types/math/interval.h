#pragma once

#include <cgv/math/interval.h>
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T>
struct interval : public cgv::math::interval<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return 
			rh.reflect_member("lb", this->lb) &&
			rh.reflect_member("ub", this->ub);
	}
};
		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace math {
#endif

		template <typename T>
		cgv::reflect::extern_reflection_traits<cgv::math::interval<T>, cgv::reflect::math::interval<T> > 
			get_reflection_traits(const cgv::math::interval<T>&) { 
				return cgv::reflect::extern_reflection_traits<cgv::math::interval<T>, cgv::reflect::math::interval<T> >();
		}

	}
}
