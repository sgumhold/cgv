#pragma once

#include <cgv/math/thin_plate_spline.h>
#include "vec.h"
#include "mat.h"
#include <cgv/reflect/reflect_extern.h>

namespace cgv {
	namespace reflect {
		namespace math {

template <typename T>
struct thin_plate_spline : public cgv::math::thin_plate_spline<T>
{
	bool self_reflect(cgv::reflect::reflection_handler& rh) {
		return 
			rh.reflect_member("controlpoints", this->controlpoints) &&
			rh.reflect_member("weights", this->weights) &&
			rh.reflect_member("affine_transformation", this->affine_transformation);
	}
};
		}

#ifdef REFLECT_IN_CLASS_NAMESPACE
}} namespace cgv { namespace math {
#endif

template <typename T>
cgv::reflect::extern_reflection_traits<cgv::math::thin_plate_spline<T>, cgv::reflect::math::thin_plate_spline<T> > get_reflection_traits(const cgv::math::thin_plate_spline<T>&) { return cgv::reflect::extern_reflection_traits<cgv::math::thin_plate_spline<T>, cgv::reflect::math::thin_plate_spline<T> >(); }

	}
}
