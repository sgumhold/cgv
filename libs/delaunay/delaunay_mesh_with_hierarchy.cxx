#include "delaunay_mesh_with_hierarchy.h"
#include <iostream>
#include <algorithm>
#include <math.h>

/// construct empty triangle mesh
template <class T>
delaunay_mesh_with_hierarchy<T>::delaunay_mesh_with_hierarchy() 
{
	hierarchy_factor = 32;
	next_hierarchy = hierarchy_factor;
}

/// copy constructor
template <class T>
delaunay_mesh_with_hierarchy<T>::delaunay_mesh_with_hierarchy(const delaunay_mesh_with_hierarchy& dm)
{
	hierarchy_factor = dm.hierarchy_factor;
	next_hierarchy = dm.next_hierarchy;
	*((delaunay_mesh_type*)this) = dm;
	H.resize(dm.H.size());
	for (unsigned int i=0; i<H.size(); ++i) {
		H[i] = new hierarchy_level_type(*dm.H[i]);
		H[i]->set_reference_geometry(this);
	}
}

/// construct empty triangle mesh
template <class T>
void delaunay_mesh_with_hierarchy<T>::clear() 
{
	delaunay_mesh_type::clear();
	for (unsigned int i=0; i<H.size(); ++i)
		delete H[i];
	H.clear();
	next_hierarchy = hierarchy_factor;
}

///
template <class T>
void delaunay_mesh_with_hierarchy<T>::set_hierarchy_factor(unsigned int hf) 
{
	hierarchy_factor = hf; 	
	next_hierarchy = hierarchy_factor;
}

/// configure the hierarchy settings for the given number of points or if not specified for the current number of points
template <class T>
void delaunay_mesh_with_hierarchy<T>::configure_hierarchy(unsigned int n)
{
	if (n == -1)
		n = T::get_nr_vertices();
	unsigned int fk = 1;
	while (fk * hierarchy_factor * hierarchy_factor < n)
		fk *= hierarchy_factor;
	next_hierarchy = (n/fk)+1;
}

/// overload point localization to use hierarchy
template <class T>
typename delaunay_mesh_with_hierarchy<T>::point_location_info delaunay_mesh_with_hierarchy<T>::localize_point(
	const point_type& p, unsigned int) const
{
	unsigned int vi = 0;
	for (unsigned int hi = 0; hi < H.size(); ++hi)
		vi = H[hi]->find_nearest_neighbor(p, H[hi]->ci_of_vi(vi));
	return delaunay_mesh_type::triangle_mesh_type::localize_point(p, T::ci_of_vi(vi));
}

/// return index of the nearest neighbor of the given point
template <class T>
unsigned int delaunay_mesh_with_hierarchy<T>::find_nearest_neighbor(const point_type& p, unsigned int) const
{
	unsigned int vi = 0;
	for (unsigned int hi = 0; hi < H.size(); ++hi)
		vi = H[hi]->find_nearest_neighbor(p, H[hi]->ci_of_vi(vi));
	return delaunay_mesh_type::find_nearest_neighbor(p, T::ci_of_vi(vi));
}

/// insert a vertex by keeping a delaunay triangulation. If a vertex with the same location already exists, ignore vertex and return index of vertex with identical location
template <class T>
typename delaunay_mesh_with_hierarchy<T>::vertex_insertion_info delaunay_mesh_with_hierarchy<T>::insert_vertex(unsigned int vi, unsigned int ci_start, std::vector<unsigned int>* touched_corners)
{
	vertex_insertion_info vii = delaunay_mesh_type::insert_vertex(vi,ci_start,touched_corners);
	if (vii.insert_error)
		return vii;

	if (vi == next_hierarchy) {
		next_hierarchy *= hierarchy_factor;
		hierarchy_level_type* h = new hierarchy_level_type();
		h->set_reference_geometry(this);
		*((typename delaunay_mesh_type::triangle_mesh_type::connectivity_type*)h) = *this;
		H.push_back(h);
	}
	return vii;
}