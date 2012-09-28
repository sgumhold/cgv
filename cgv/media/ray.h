#pragma once

#include <cgv/math/vec.h>

namespace cgv {
	namespace media {

/**
* A simple ray data structure for arbitrary dimensions. The ray is defined by 
  its origin point and a direction.
*/
template<class T>
class ray
{
public:
	cgv::math::vec<T> origin;
	cgv::math::vec<T> direction;
	
	ray()
	{
	}

	///create a ray 
	ray(const cgv::math::vec<T>& o,const cgv::math::vec<T> &d)
	{
		origin=o;
		direction=d;
	}
};

	}
}

