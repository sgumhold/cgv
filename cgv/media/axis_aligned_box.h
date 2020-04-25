#pragma once

#include <cgv/type/standard_types.h>
#include <cgv/math/vec.h>
#include <cgv/math/fvec.h>

namespace cgv {
	namespace media {

/**
 * An axis aligned box, defined by to points: min and max
 */
template<typename T, cgv::type::uint32_type N>
class axis_aligned_box
{
public:
	/// internally fixed sized points and vectors are used
	typedef cgv::math::fvec<T,N> fpnt_type;
	typedef cgv::math::fvec<T,N> fvec_type;
	/// the interface allows also to work with variable sized points
	typedef cgv::math::vec<T> pnt_type;
	typedef cgv::math::vec<T> vec_type;
protected:
	fpnt_type minp;
	fpnt_type maxp;
public:
	/// standard constructor does not initialize
	axis_aligned_box() { invalidate(); }
	/// type conversion copy constructor
	template <typename S>
	axis_aligned_box(const axis_aligned_box<S, N>& B) : 
		minp(T(B.get_min_pnt()(0)), T(B.get_min_pnt()(1)), T(B.get_min_pnt()(2))), 
		maxp(T(B.get_max_pnt()(0)), T(B.get_max_pnt()(1)), T(B.get_max_pnt()(2))) {}
	/// construct from min point and max point
	axis_aligned_box(const fpnt_type& _minp, const fpnt_type& _maxp) : minp(_minp), maxp(_maxp) {}
	/// construct from min point and max point
	axis_aligned_box(const pnt_type& _minp, const pnt_type& _maxp)
	{
		invalidate();
		unsigned i;
		for (i=0; i<_minp.size(); ++i) {
			if (i == N)
				break;
			minp(i) = _minp(i);
		}
		for (i=0; i<_maxp.size(); ++i) {
			if (i == N)
				break;
			maxp(i) = _maxp(i);
		}
	}
	/// set to invalid min and max points
	void invalidate() {
		for (unsigned int c = 0; c<N; ++c) {
			minp(c) = 1;
			maxp(c) = 0;
		}
	}
	/// return a const reference to corner 0
	const fpnt_type& get_min_pnt() const { return minp; }
	/// return a const reference to corner 7
	const fpnt_type& get_max_pnt() const { return maxp; }
	/// return a reference to corner 0
	fpnt_type& ref_min_pnt() { return minp; }
	/// return a reference to corner 7
	fpnt_type& ref_max_pnt() { return maxp; }
	/// return the i-th corner where the lower N bits of i are used to select between min (bit = 0) and max (bit = 1) coordinate
	fpnt_type get_corner(int i) const
	{
		fpnt_type c = minp;
		int bit = 1;
		for (unsigned int dim=0; dim < N; ++dim, bit *= 2)
			if (i & bit)
				c(dim) = maxp(dim);
		return c;
	}
	/// return a vector with the extents in the different dimensions
	fvec_type get_extent() const { return maxp-minp; }
	/// return the center of the box
	fpnt_type get_center() const { return (minp+maxp)/2; }
	/// check if aab is valid
	bool is_valid() const { return minp[0] <= maxp[0]; }
	/// check whether a point is inside the aabb
	bool inside(const fpnt_type& p) const {
		for (unsigned int c = 0; c<N; ++c) {
			if (p(c) < minp(c)) return false;
			if (p(c) >= maxp(c)) return false;
		}
		return true;
	}
	/// extent box to include given point
	void add_point(const fpnt_type& p) {
		if (is_valid()) {
			for (unsigned int c = 0; c<N; ++c) {
				if (p(c) > maxp(c)) maxp(c) = p(c);
				if (p(c) < minp(c)) minp(c) = p(c);
			}
		}
		else
			minp = maxp = p;
	}
	/// extent box to include given point
	void add_point(const pnt_type& p) {
		if (is_valid()) {
			for (unsigned int c = 0; p.size(); ++c) {
				if (c == N)
					break;
				if (p(c) > maxp(c)) maxp(c) = p(c);
				if (p(c) < minp(c)) minp(c) = p(c);
			}
		}
		else
			*this = axis_aligned_box<T,N>(p,p);
	}
	/// extent box to include given axis alinged box
	void add_axis_aligned_box(const axis_aligned_box<T,N>& aab) {
		if (!aab.is_valid())
			return;
		add_point(aab.minp);
		add_point(aab.maxp);
	}
	/// scale the complete box with respect to the world coordinate origin
	void scale(const T& f)
	{
		if (!is_valid())
			return;
		for (unsigned int c = 0; c<N; ++c) {
			maxp(c) *= f;
			minp(c) *= f;
		}
	}
	/// translate box by vector
	void translate(const fpnt_type& v)
	{
		if (!is_valid())
			return;
		ref_min_pnt() += v;
		ref_max_pnt() += v;
	}
	/// return the index of the coordinte with maximum extend
	unsigned get_max_extent_coord_index() const
	{
		fvec_type e = get_extent();
		unsigned j = 0;
		for (unsigned i=1; i<N; ++i)
			if (e(i) > e(j))
				j = i;
		return j;
	}
};

/// stream out of a box
template<typename T, cgv::type::uint32_type N>
std::ostream& operator<<(std::ostream& out, const axis_aligned_box<T, N>& box)
{
	return out << box.get_min_pnt() << "->" << box.get_max_pnt();
}


	}
}

