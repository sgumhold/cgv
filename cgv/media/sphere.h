#pragma once

#include <cgv/math/vec.h>

namespace cgv {
	namespace media {

/**
 * A sphere data structure. The sphere is defined by a homogeneous vector
   which keeps the center point in the first components and the radius in
	the last component.
 */
template <typename T,int N>
class sphere
{
public:
	cgv::math::vec<T> h;
	///create a unit sphere
	sphere() : h(0,0,0,1)
	{
	}
	/// construct from center and radius
	sphere(const cgv::math::vec<T>& c, const T& radius)
	{
		h.resize(c.size()+1);
		for (unsigned int i=0; i<c.size(); ++i)
			h(i) = c(i);
		h(c.size()) = radius;
	}
};

	}
}

