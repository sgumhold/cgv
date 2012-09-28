#pragma once

#include <cgv/math/vec.h>

namespace cgv {
	namespace media {

/**
* A plane of arbitrary dimension stored as a homogeneous vector that
  defines the coefficients of the plane equation.
*/
template <class T>
class plane
{
public:
	cgv::math::vec<T> h;
	/// construct plane of give dimension allocating a homogenous vector of one dimension mor
	plane(int dim = 3)
	{
		h.resize(dim+1); 
	}
	/// construct from normal vector and distance of plane to origin, which 
	/// is negated before it is copied to the last component of the stored 
	/// homogenous vector
	plane(const cgv::math::vec<T> &n, const T &distance)
	{
		h.resize(n.size()+1);
		for (unsigned int i=0; i<n.size(); ++i)
			h(i) = n(i);
		h(n.size()) = -distance;
	}
	/// return the normal vector
	cgv::math::vec<T> get_normal() const
	{
		cgv::math::vec<T> n;
		n.resize(h.size()-1);
		for (unsigned int i=0; i<n.size(); ++i)
			n(i) = h(i);
		return n;
	}
	/// return the distance of the plane to the origin
	T get_distance() const
	{
		return -h(h.size()-1);
	}
};

	}
}

