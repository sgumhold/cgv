#include "corner_connectivity.h"
#include <iostream>

/// construct empty triangle mesh
corner_connectivity::corner_connectivity() 
{
}

/// remove all triangles
void corner_connectivity::clear_triangles() 
{ 
	C.clear(); 
}

/// add a triangle with all edges boundary edges
void corner_connectivity::add_triangle(unsigned int v0, unsigned int v1, unsigned int v2)
{
	add_triangle(corner(v0),corner(v1),corner(v2));
}

/// add a triangle with the given cornern information
void corner_connectivity::add_triangle(const corner& c0, const corner& c1, const corner& c2)
{
	C.push_back(c0);
	C.push_back(c1);
	C.push_back(c2);
}

/// check if an edge is flipable
bool corner_connectivity::is_flipable(unsigned int c0) const
{
	// check if opposite vertex is available
	if (is_opposite_to_border(c0))
		return false;
	unsigned int vo = vi_of_ci(inv(c0));
	// next check if the to be generated edge is already present in the triangulation
	neighbor_cycler nc = get_nbr_cycler(c0);
	do {
		if (vi_of_ci(nc.ci_of_nbr()) == vo)
			return false;
		nc.next();
	} while (!nc.cycle_complete());
	return true;
}


/// flip an edge that is opposite to a corner c0
void corner_connectivity::flip_edge(unsigned int c0)
{
	if (is_opposite_to_border(c0)) {
		std::cerr << "attempt to call flip_edge on a corner opposite to the border" << std::endl;
		return;
	}
	unsigned int c1 = next(c0);
	unsigned int c2 = next(c1);
	unsigned int c3 = inv(c0);
	unsigned int c4 = next(c3);
	unsigned int c5 = next(c4);
	vi_of_ci(c2) = vi_of_ci(c3);
	vi_of_ci(c5) = vi_of_ci(c0);
	const opposite_info& o4 = opposite(c4);
	opposite(c0) = o4;
	if (!o4.is_opposite_to_border)
		opposite(o4.ci) = opposite_info(c0);
	const opposite_info& o1 = opposite(c1);
	opposite(c3) = o1;
	if (!o1.is_opposite_to_border)
		opposite(o1.ci) = opposite_info(c3);
	opposite(c1) = opposite_info(c4);
	opposite(c4) = opposite_info(c1);
}
/// split a triangle given by a corner into 3 triangles at the given vertex index
void corner_connectivity::split_triangle_at_vertex(unsigned int c0, unsigned int vi)
{
	unsigned int c1 = next(c0);
	unsigned int c2 = next(c1);
	unsigned int v0 = vi_of_ci(c0);
	unsigned int v1 = vi_of_ci(c1);
	unsigned int v2 = vi_of_ci(c2);
	unsigned int cn = get_nr_corners();
	add_triangle(corner(vi, opposite(c2)),corner(v0, c2),corner(v1, cn+4));
	add_triangle(corner(vi, opposite(c1)),corner(v2, cn+2),corner(v0, c1));
	vi_of_ci(c0) = vi;
	opposite(c1) = opposite_info(cn+5);
	opposite(c2) = opposite_info(cn+1);
	if (!is_opposite_to_border(cn))
		opposite(inv(cn)) = opposite_info(cn);
	if (!is_opposite_to_border(cn+3))
		opposite(inv(cn+3)) = opposite_info(cn+3);
}
/// build a triangle with vertex vi on the border edge specified by the opposite corner c0
void corner_connectivity::build_triangle_on_border_edge(unsigned int c0, unsigned int vi)
{
//	std::cout << "   build_triangle_on_border_edge(" << c0 << "," << vi << ")" << std::endl;
	unsigned int c1 = next(c0);
	unsigned int c2 = next(c1);
	unsigned int v0 = vi_of_ci(c0);
	unsigned int v1 = vi_of_ci(c1);
	unsigned int v2 = vi_of_ci(c2);
	unsigned int cn = get_nr_corners();
	add_triangle(corner(vi,c0),corner(v2),corner(v1));
	opposite(c0) = opposite_info(cn);
}
/// find the corner that is opposite to the border edge which follows the border edge opposite to the given corner index
unsigned int corner_connectivity::next_on_border(unsigned int ci) const
{
	if (!is_opposite_to_border(ci)) {
		std::cerr << "attempt to call next_on_border with corner not opposite to border edge" << std::endl;
		return ci;
	}
	ci = prev(ci);
	while (!is_opposite_to_border(ci))
		ci = prev(inv(ci));
	return ci;
}
/// find the corner that is opposite to the border edge which preceeds the border edge opposite to the given corner index
unsigned int corner_connectivity::prev_on_border(unsigned int ci) const
{
	if (!is_opposite_to_border(ci)) {
		std::cerr << "attempt to call prev_on_border with corner not opposite to border edge" << std::endl;
		return ci;
	}
	ci = next(ci);
	while (!is_opposite_to_border(ci))
		ci = next(inv(ci));
	return ci;
}
/// build a triangle in between the border edge opposite to the given corner and the next edge along the border
void corner_connectivity::build_triangle_connection_to_next_border_edge(unsigned int c0)
{
	if (!is_opposite_to_border(c0)) {
		std::cerr << "attempt to call build_triangle_connection_to_next_border_edge with corner not opposite to border edge" << std::endl;
		return;
	}
//	std::cout << "   build_triangle_connection_to_next_border_edge(" << c0 << ")" << std::endl;
	unsigned int c1 = next(c0);
	unsigned int c2 = next(c1);
	unsigned int v0 = vi_of_ci(c0);
	unsigned int v1 = vi_of_ci(c1);
	unsigned int v2 = vi_of_ci(c2);
	unsigned int cb = next_on_border(c0);
	unsigned int ca = next(cb);
	unsigned int va = vi_of_ci(ca);
	unsigned int cn = get_nr_corners();
	C.push_back(corner(va,c0));
	C.push_back(corner(v2,cb));
	C.push_back(corner(v1));
	opposite(c0) = opposite_info(cn);
	opposite(cb) = opposite_info(cn+1);
}
/// build a triangle in between the border edge opposite to the given corner and the previous edge along the border
void corner_connectivity::build_triangle_connection_to_prev_border_edge(unsigned int c0)
{
	if (!is_opposite_to_border(c0)) {
		std::cerr << "attempt to call build_triangle_connection_to_prev_border_edge with corner not opposite to border edge" << std::endl;
		return;
	}
//	std::cout << "   build_triangle_connection_to_prev_border_edge(" << c0 << ")" << std::endl;
	unsigned int c1 = next(c0);
	unsigned int c2 = next(c1);
	unsigned int v0 = vi_of_ci(c0);
	unsigned int v1 = vi_of_ci(c1);
	unsigned int v2 = vi_of_ci(c2);
	unsigned int cb = prev_on_border(c0);
	unsigned int ca = prev(cb);
	unsigned int va = vi_of_ci(ca);
	unsigned int cn = get_nr_corners();
	C.push_back(corner(va,c0));
	C.push_back(corner(v2));
	C.push_back(corner(v1,cb));
	opposite(c0) = opposite_info(cn);
	opposite(cb) = opposite_info(cn+2);
}
