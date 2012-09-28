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
			rh.reflect_member("controlpoints", controlpoints) &&
			rh.reflect_member("weights", weights) &&
			rh.reflect_member("affine_transformation", affine_transformation);
	}
};
		}

template <typename T>
extern_reflection_traits<cgv::math::thin_plate_spline<T>, cgv::reflect::math::thin_plate_spline<T> > get_reflection_traits(const cgv::math::thin_plate_spline<T>&) { return extern_reflection_traits<cgv::math::thin_plate_spline<T>, cgv::reflect::math::thin_plate_spline<T> >(); }

	}
}
