
// C++ STL
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <utility>

// implemented header
#include "regulargrid.h"


////
// Class implementation

template<class flt_type>
struct grid3D<flt_type>::Impl
{
	// types
	typedef flt_type real;
	typedef typename grid3D::Vec3 Vec3;
	typename grid3D::point_accessor pnt_access;

	// fields
	real cellwidth;
	std::unordered_map<
		typename grid3D::cell, std::vector<size_t>,
		typename grid3D::cell::hash
	> grid;

	// helper methods
	Impl(real cellwidth, const grid3D::point_accessor &point_accessor)
		: cellwidth(cellwidth), pnt_access(point_accessor)
	{}
};

template <class flt_type>
grid3D<flt_type>::grid3D(real cellwidth, const point_accessor &pa) : pimpl(nullptr)
{
	pimpl = new Impl(cellwidth, pa);
}

template <class flt_type>
grid3D<flt_type>::~grid3D()
{
	if (pimpl)
		delete pimpl;
}

template <class flt_type>
void grid3D<flt_type>::insert (size_t index)
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// retrieve the point
	Vec3 point;
	impl.pnt_access(&point, index);

	// insert into appropriate cell
	cell c = cell::get(point, impl.cellwidth);
	impl.grid[c].push_back(index);
}

template <class flt_type>
bool grid3D<flt_type>::query (std::vector<size_t> *out, const Vec3 &query_point, bool distance_sort) const
{
	// shortcut for saving one indirection
	auto &impl = *pimpl;

	// keep track of first new element position in output array
	size_t sort_start = out->size();

	// determine query cell
	cell qc = cell::get(query_point, impl.cellwidth);

	// loop through 27-neighborhood
	bool found = false;
	for (unsigned i=0; i<27; i++)
	{
		cell q_cur{/* x = */ qc.x-1 + long(i%3),
		           /* y = */ qc.y-1 + long((i/3)%3),
		           /* z = */ qc.z-1 + long(i/9)};
		auto c = impl.grid.find(q_cur);
		if (c != impl.grid.end())
		{
			found = true;
			out->insert(out->end(), c->second.begin(), c->second.end());
		}
	}

	// Sort according to distance if requested
	if (distance_sort)
		std::sort(out->begin()+sort_start, out->end(),
		[&impl, &query_point] (size_t l, size_t r) {
			Vec3 lpoint, rpoint;
			impl.pnt_access(&lpoint, l);
			impl.pnt_access(&rpoint, r);
			return   (lpoint - query_point).sqr_length()
			       < (rpoint - query_point).sqr_length();
		});

	return found;
}

template <class flt_type>
void grid3D<flt_type>::set_point_accessor (const point_accessor &pa)
{
	pimpl->pnt_access = pa;
}



//////
//
// Explicit template instantiations
//

// Only float and double variants are intended
template class grid3D<float>;
template class grid3D<double>;
