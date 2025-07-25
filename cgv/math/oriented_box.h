#pragma once

#include "fvec.h"
#include "quaternion.h"

namespace cgv {
namespace math {

template<typename T>
struct oriented_box2 {
	fvec<T, 2> center;
	fvec<T, 2> extent;
	T rotation;
};

template<typename T>
struct oriented_box3 {
	fvec<T, 3> center;
	fvec<T, 3> extent;
	quaternion<T> rotation;
};

} // namespace math
} // namespace cgv
