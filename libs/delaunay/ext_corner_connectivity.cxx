#include "ext_corner_connectivity.h"
#include <iostream>

/// construct empty triangle mesh
ext_corner_connectivity::ext_corner_connectivity() 
{
}

/// remove all triangles
void ext_corner_connectivity::clear_triangles() 
{ 
	C.clear(); 
	V.clear();
}

/// add a triangle with all edges boundary edges
void ext_corner_connectivity::add_triangle(unsigned int v0, unsigned int v1, unsigned int v2)
{
	add_triangle(corner(v0),corner(v1),corner(v2));
}

/// add a triangle with the given cornern information
void ext_corner_connectivity::add_triangle(const corner& c0, const corner& c1, const corner& c2)
{
	unsigned int cn = get_nr_corners();
	corner_connectivity::add_triangle(c0,c1,c2);
	ci_of_vi(c0.vi) = cn;
	ci_of_vi(c1.vi) = cn+1;
	ci_of_vi(c2.vi) = cn+2;
}

/// check if the ci_of_vi field is consistent
void ext_corner_connectivity::is_ci_of_vi_consistent(unsigned int n) const
{
	bool found_error = false;
	for (unsigned int vi = 0; vi < n; ++vi) {
		if (vi_of_ci(ci_of_vi(vi)) != vi) {
			std::cerr << "vi_of_ci_of_vi(" << vi << ") = " << vi_of_ci(ci_of_vi(vi)) << "!!!" << std::endl;
			found_error = true;
		}
	}
	if (!found_error)
		std::cerr << "ci_of_vi is ok" << std::endl;
}


/// flip an edge that is opposite to a corner c0
void ext_corner_connectivity::flip_edge(unsigned int c0)
{
	corner_connectivity::flip_edge(c0);
	if (is_opposite_to_border(c0))
		return;
	unsigned int c1 = next(c0);
	ci_of_vi(vi_of_ci(c1)) = c1;
	unsigned int c4 = inv(c1);
	ci_of_vi(vi_of_ci(c4)) = c4;
}
/// split a triangle given by a corner into 3 triangles at the given vertex index
void ext_corner_connectivity::split_triangle_at_vertex(unsigned int c0, unsigned int vi)
{
	unsigned int cn = get_nr_corners();
	unsigned int v0 = vi_of_ci(c0);
	unsigned int v1 = vi_of_ci(next(c0));
	unsigned int v2 = vi_of_ci(prev(c0));
	corner_connectivity::split_triangle_at_vertex(c0,vi);
	ci_of_vi(v0) = cn+1;
	ci_of_vi(v1) = cn+2;
	ci_of_vi(v2) = cn+4;
	ci_of_vi(vi) = cn;
}

/// build a triangle with vertex vi on the border edge specified by the opposite corner c0
void ext_corner_connectivity::build_triangle_on_border_edge(unsigned int c0, unsigned int vi)
{
	unsigned int cn = get_nr_corners();
	corner_connectivity::build_triangle_on_border_edge(c0,vi);
	ci_of_vi(vi) = cn;
}
