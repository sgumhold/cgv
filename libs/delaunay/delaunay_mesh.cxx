#include "delaunay_mesh.h"
#include <iostream>
#include <algorithm>
#include <math.h>

/// construct empty triangle mesh
template <class T>
delaunay_mesh<T>::delaunay_mesh() 
{
}

/// return index of the nearest neighbor of the given point
template <class T>
unsigned int delaunay_mesh<T>::find_nearest_neighbor(const point_type& p, unsigned int ci_start) const
{
	if (this->C.empty()) {
		std::cerr << "cannot find a nearest neighbor because no triangle has been specified" << std::endl;
		return -1;
	}
	// choose some ci
	unsigned int ci = ci_start;
	coord_type min_squared_dist = T::sqr_dist(p,T::p_of_vi(T::vi_of_ci(ci)));
	while (true) {
		unsigned int cj = ci;
		unsigned int ck = -1;
		bool jump_boundary = false;
		coord_type squared_dist = -1;
		do {
			coord_type new_squared_dist = T::sqr_dist(p,T::p_of_vi(T::vi_of_ci(T::next(cj))));
			if (squared_dist == -1 || new_squared_dist < squared_dist) {
				squared_dist = new_squared_dist;
				ck = T::next(cj);
			}
			if (jump_boundary) {
				cj = T::next(T::prev_on_border(cj));
				jump_boundary = false;
			}
			else
				if (T::is_opposite_to_border(T::next(cj))) {
					cj = T::next(cj);
					jump_boundary = true;
				}
				else
					cj = T::next(T::inv(T::next(cj)));
		} while (jump_boundary || cj != ci);

		if (min_squared_dist <= squared_dist)
			break;
		ci = ck;
		min_squared_dist = squared_dist;
	}
	return T::vi_of_ci(ci);
}


/// check if the edge opposite to the given corner is locally delaunay
template <class T>
bool delaunay_mesh<T>::is_locally_delaunay(unsigned int ci) const
{
	const point_type& p0 = T::p_of_vi(T::vi_of_ci(ci));
	const point_type& p1 = T::p_of_vi(T::vi_of_ci(T::next(ci)));
	const point_type& p2 = T::p_of_vi(T::vi_of_ci(T::prev(ci)));
	const point_type& p3 = T::p_of_vi(T::vi_of_ci(T::inv(ci)));
	const coord_type& x0 = p0.x();
	const coord_type& y0 = p0.y();
	      coord_type  r0 = x0*x0+y0*y0;
	const coord_type& x1 = p1.x();
	const coord_type& y1 = p1.y();
	      coord_type  r1 = x1*x1+y1*y1;
	const coord_type& x2 = p2.x();
	const coord_type& y2 = p2.y();
	      coord_type  r2 = x2*x2+y2*y2;
	const coord_type& x3 = p3.x();
	const coord_type& y3 = p3.y();
	      coord_type  r3 = x3*x3+y3*y3;
	coord_type det = 
		x0 * ( y1 * r2 
	        + y2 * r3 
	        + y3 * r1 )
	 + x1 * ( y0 * r3 
	        + y2 * r0 
	        + y3 * r2 )
	 + x2 * ( y0 * r1 
	        + y1 * r3 
	        + y3 * r0 )
	 + x3 * ( y0 * r2 
	        + y1 * r0 
	        + y2 * r1 )
	 - (
	   x0 * ( y1 * r3 
	        + y2 * r1 
	        + y3 * r2 )
	 + x1 * ( y0 * r2 
	        + y2 * r3 
	        + y3 * r0 )
	 + x2 * ( y0 * r3 
	        + y1 * r0 
	        + y3 * r1 )
	 + x3 * ( y0 * r1 
	        + y1 * r2 
	        + y2 * r0 ) );
	return det < 0;
}

/// insert a vertex by keeping a delaunay triangulation. If a vertex with the same location already exists, ignore vertex and return index of vertex with identical location
template <class T>
typename delaunay_mesh<T>::vertex_insertion_info delaunay_mesh<T>::insert_vertex(unsigned int vi, unsigned int ci_start, std::vector<unsigned int>* touched_corners)
{
	unsigned int n = T::get_nr_triangles();
	vertex_insertion_info vii = triangle_mesh_type::insert_vertex(vi,ci_start);
	if (vii.insert_error || vii.is_duplicate)
		return vii;

	if (vii.extends_border) {
		n = T::get_nr_triangles()-n;
		unsigned int ci = vii.ci_of_vertex;
		while (!T::is_opposite_to_border(T::prev(ci)))
			ci = T::prev(T::inv(T::prev(ci)));
		flip_edges_around_vertex_to_validate(ci, n, touched_corners);
	}
	else {
		flip_edges_around_vertex_to_validate(vii.ci_of_vertex, 3, touched_corners);
	}
	return vii;
}

/// flip the edges in the one ring of the corner vertex until all are locally delaunay
template <class T>
void delaunay_mesh<T>::flip_edges_around_vertex_to_validate(unsigned int ci, unsigned int n, std::vector<unsigned int>* touched_corners)
{
	unsigned int c0 = ci;
	while (n > 0) {
		if (T::is_flipable(ci) && !is_locally_delaunay(ci)) {
			T::flip_edge(ci);
			++n;
		}
		else {
			if (touched_corners)
				touched_corners->push_back(ci);
			ci = T::next(T::inv(T::next(ci)));
			if (ci == c0)
				break;
			--n;
		}
	}
}
