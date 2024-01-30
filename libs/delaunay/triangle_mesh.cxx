#include "epsilon.h"
#include "triangle_mesh.h"
#include "mesh_geometry_reference.h"

#include <iostream>
#include <algorithm>
#include <cmath>

/// construct empty triangle mesh
template <class G, class C>
triangle_mesh<G,C>::triangle_mesh()
{
}

/// construct empty triangle mesh
template <class G, class C>
triangle_mesh<G,C>::~triangle_mesh()
{
	clear();
}


/// remove all vertices and triangles
template <class G, class C>
void triangle_mesh<G,C>::clear()
{
	C::clear_triangles();
	G::clear_geometry();
}

/// add a triangle with all edges boundary edges
template <class G, class C>
void triangle_mesh<G,C>::add_triangle(unsigned int v0, unsigned int v1, unsigned int v2)
{
	if (geometry_type::is_outside(G::p_of_vi(v0),G::p_of_vi(v1),G::p_of_vi(v2)))
		connectivity_type::add_triangle(corner(v1),corner(v0),corner(v2));
	else
		connectivity_type::add_triangle(corner(v0),corner(v1),corner(v2));
}

/// add a triangle with the given cornern information
template <class G, class C>
void triangle_mesh<G,C>::add_triangle(const corner& c0, const corner& c1, const corner& c2)
{
	if (geometry_type::is_outside(G::p_of_vi(c0.vi),G::p_of_vi(c1.vi),G::p_of_vi(c2.vi)))
		connectivity_type::add_triangle(c1,c0,c2);
	else
		connectivity_type::add_triangle(c0,c1,c2);
}

///
template <class G, class C>
bool triangle_mesh<G,C>::is_outside(const point_type& p, unsigned int c0) const
{
	unsigned int c1 = C::next(c0);
	unsigned int c2 = C::next(c1);
	return geometry_type::is_outside(p, G::p_of_vi(C::vi_of_ci(c1)), G::p_of_vi(C::vi_of_ci(c2)));
}

///
template <class G, class C>
bool triangle_mesh<G,C>::is_inside(const point_type& p, unsigned int c0) const
{
	unsigned int c1 = C::prev(c0);
	unsigned int c2 = C::prev(c1);
	return geometry_type::is_outside(p, G::p_of_vi(C::vi_of_ci(c1)), G::p_of_vi(C::vi_of_ci(c2)));
}

/// extend flipable predicate by geometric considerations
template <class G, class C>
bool triangle_mesh<G,C>::is_flipable(unsigned int c0) const
{
	if (!corner_connectivity::is_flipable(c0))
		return false;
	const point_type& p0 = G::p_of_vi(C::vi_of_ci(c0));
	const point_type& p1 = G::p_of_vi(C::vi_of_ci(C::next(c0)));
	const point_type& p2 = G::p_of_vi(C::vi_of_ci(C::prev(c0)));
	const point_type& p3 = G::p_of_vi(C::vi_of_ci(C::inv(c0)));

	bool in_1_03 = geometry_type::is_outside(p1,p0,p3);
	bool in_2_30 = geometry_type::is_outside(p2,p3,p0);
	if (in_1_03 != in_2_30)
		return true;

	bool in_1_30 = geometry_type::is_outside(p1,p3,p0);
	if (in_1_30 && in_2_30) {
		std::cout << "disallowed flip due to geometric constraints" << std::endl;
		return false;
	}
	bool in_2_03 = geometry_type::is_outside(p2,p0,p3);
	if (in_1_03 && in_2_03) {
		std::cout << "disallowed flip due to geometric constraints" << std::endl;
		return false;
	}
	return true;
}


/// find the triangle containing p or the corner opposite to a border edge outside of which the p lies
template <class G, class C>
typename triangle_mesh<G,C>::point_location_info
	triangle_mesh<G,C>::localize_point(const point_type& p, unsigned int start_ci) const
{
	point_location_info pli;
	if (this->C.empty()) {
		std::cerr << "cannot locate the given point because no triangle has been specified" << std::endl;
		pli.is_outside = true;
		pli.ci = /*-1*/0x7fffffff; // <-- using "-1" instead of this bitpattern would be more portable, but causes unavoidable overflow warnings in some compilers (including GCC and Clang)
		return pli;
	}
	unsigned int ci = start_ci;
	unsigned int k = 3;
	while (true) {
		unsigned int border_exit_ci = -1;
		unsigned int i;
		for (i=0; i<k; ++i) {
			bool is_border = C::is_opposite_to_border(ci);
			if (is_outside(p, ci)) {
				if (is_border) {
					pli.is_outside = true;
					pli.ci = ci;
					return pli;
				}
				else {
					ci = C::next(C::inv(ci));
					border_exit_ci = -1;
					break;
				}
			}
			else if (is_border) {
				if (!is_inside(p,ci))
					border_exit_ci = ci;
			}
			ci = C::next(ci);
		}
		if (border_exit_ci != -1) {
			pli.is_outside = true;
			pli.ci = border_exit_ci;
			return pli;
		}
		if (i == k) {
			pli.is_outside = false;
			pli.ci = ci;
			return pli;
		}
		k = 2;
	}
}

/// insert
template <class G, class C>
void triangle_mesh<G,C>::build_convex_fan_on_border(unsigned int ci, unsigned int vi)
{
//	std::cout << "build_convex_fan_on_border(" << ci << "," << vi << ")" << std::endl;
	C::build_triangle_on_border_edge(ci,vi);
	unsigned int cj = ci;
	while (true) {
		cj = C::next(C::inv(cj));
		unsigned int cb = C::next_on_border(cj);
		unsigned int vb = C::vi_of_ci(C::next(cb));
//		std::cout << "   check vertex " << vb << " to edge (" << vi_of_ci(next(cj)) << "," << vi_of_ci(prev(cj)) << ")" << std::endl;
		if (!is_outside(G::p_of_vi(vb), cj))
			break;
		C::build_triangle_connection_to_next_border_edge(cj);
		cj = cb;
	}
	cj = ci;
	while (true) {
		cj = C::prev(C::inv(cj));
		unsigned int cb = C::prev_on_border(cj);
		unsigned int vb = C::vi_of_ci(C::prev(cb));
//		std::cout << "   check vertex " << vb << " to edge (" << vi_of_ci(next(cj)) << "," << vi_of_ci(prev(cj)) << ")" << std::endl;
		if (!is_outside(G::p_of_vi(vb), cj))
			break;
		C::build_triangle_connection_to_prev_border_edge(cj);
		cj = cb;
	}
}

template <class G, class C>
typename triangle_mesh<G,C>::vertex_insertion_info triangle_mesh<G,C>::insert_vertex(unsigned int vi, unsigned int start_ci)
{
	vertex_insertion_info vii;
	vii.is_duplicate = false;
	vii.extends_border = false;
	vii.ci_of_vertex = -1;
	if (C::get_nr_triangles() == 0) {
		std::cerr << "cannot insert a vertex without any initial triangles" << std::endl;
		vii.insert_error = true;
		return vii;
	}
	vii.insert_error = false;
	const point_type& p = G::p_of_vi(vi);
	point_location_info pli = localize_point(p, start_ci);
	vii.ci_of_vertex = pli.ci;
	if (pli.is_outside) {
		build_convex_fan_on_border(pli.ci, vi);
		vii.extends_border = true;
		vii.ci_of_vertex = C::inv(pli.ci);
	}
	else {
		const point_type& p  = G::p_of_vi(vi);
		const point_type& p0 = G::p_of_vi(C::vi_of_ci(pli.ci));
		const point_type& p1 = G::p_of_vi(C::vi_of_ci(C::next(pli.ci)));
		const point_type& p2 = G::p_of_vi(C::vi_of_ci(C::prev(pli.ci)));
		typename G::coord_type eps = epsilon<typename G::coord_type>::get_eps()*std::min(std::min(G::sqr_dist(p0,p1),G::sqr_dist(p1,p2)),G::sqr_dist(p0,p2));
		if (G::sqr_dist(p, p0) < eps) {
			vii.is_duplicate = true;
		}
		else if (G::sqr_dist(p, p1) < eps) {
			vii.is_duplicate = true;
			vii.ci_of_vertex = C::next(pli.ci);
		}
		else if (G::sqr_dist(p, p2) < eps) {
			vii.is_duplicate = true;
			vii.ci_of_vertex = C::prev(pli.ci);
		}
		else  {
			C::split_triangle_at_vertex(pli.ci, vi);
			vii.ci_of_vertex = pli.ci;
		}
	}
	return vii;
}

template <class G, class C>
void triangle_mesh<G,C>::compute_triangulation()
{
	if (G::get_nr_vertices() < 3)
		return;
	add_triangle(0,1,2);
	for (unsigned int vi=3; vi < G::get_nr_vertices(); ++vi)
		insert_vertex(vi);
}

/// collect the mesh elements that are incident to the segment between vi and vj or the edges that intersect this segment
template <class G, class C>
void triangle_mesh<G,C>::trace_segment(unsigned int vi, unsigned int vj, std::vector<trace_hit>& trace_hits) const
{
	unsigned int ci = C::ci_of_vi(vi);
	unsigned int ck = ci;
	const point_type& pi = G::p_of_vi(vi);
	const point_type& pj = G::p_of_vi(vj);
	do {
		bool trace_strip = true;
		// trace from vertex

		// first check if the target vertex is adjacent to current vertex
		typename C::neighbor_cycler nc = C::get_nbr_cycler(ck);
		for (; !nc.cycle_complete(); nc.next()) {
			if (vj == C::vi_of_ci(nc.ci_of_nbr())) {
				trace_hits.push_back(trace_hit(INCIDENT_EDGE, nc.ci_of_edge()));
				return;
			}
		}
		// check if a vertex incident edge is also incident to the segment
		const point_type& pk = G::p_of_vi(C::vi_of_ci(ck));
		for (nc = C::get_nbr_cycler(ck); !nc.cycle_complete(); nc.next()) {
			unsigned int      vn = C::vi_of_ci(nc.ci_of_nbr());
			const point_type& pn = G::p_of_vi(vn);
			if (G::is_incident(pn, pi, pj) && G::is_between(pn, pk, pj)) {
				trace_hits.push_back(trace_hit(INCIDENT_EDGE, nc.ci_of_edge()));
				trace_hits.push_back(trace_hit(INCIDENT_VERTEX, vn));
				ck = nc.ci_of_nbr();
				trace_strip = false;
				break;
			}
		}
		if (trace_strip) {
			for (typename C::corner_cycler cc = C::get_corner_cycler(ck); !cc.cycle_complete(); cc.next()) {
				// check if the segment leaves through the corner opposite edge
				ck = cc.ci();
				unsigned int cp = C::prev(ck);
				unsigned int cn = C::next(ck);
				if (is_outside(pj, ck) && is_inside(pj, cn) && is_inside(pj, cp)) {
					trace_hits.push_back(trace_hit(INTERSECTED_EDGE, ck));
					trace_strip = true;
					break;
				}
			}
			if (!trace_strip) {
				std::cerr << "could not leave vertex " << C::vi_of_ci(ck) << std::endl;
				return;
			}
		}
/*		// cycle vertex corners
		do {
			// check if the target vertex is adjacent
			unsigned int cp = prev(ck);
			unsigned int vp = vi_of_ci(cp);
			unsigned int cn = next(ck);
			unsigned int vn = vi_of_ci(cn);
			if (vj == vp) { incident_cis.push_back(cn); return; }
			if (vj == vn) { incident_cis.push_back(cp); return; }
			// check if a vertex incident edge is also incident to the segment
			const point_type& pk = p_of_vi(vi_of_ci(ck));
			const point_type& pp = p_of_vi(vp);
			const point_type& pn = p_of_vi(vn);
			if (is_incident(pp, pi, pj) && is_between(pp, pk, pj)) {
				incident_cis.push_back(cn);
				incident_vis.push_back(vp);
				ck = cp;
				trace_strip = false;
				break;
			}
			if (is_incident(pn, pi, pj) && is_between(pn, pk, pj)) {
				incident_cis.push_back(cp);
				incident_vis.push_back(vn);
				ck = cn;
				trace_strip = false;
				break;
			}
			// check if the segment leaves through the corner opposite edge
			if (is_outside(pj, ck) && is_inside(pj, cn) && is_inside(pj, cp)) {
				intersected_cis.push_back(ck);
				trace_strip = true;
				break;
			}
			// jump to next corner
			if (is_opposite_to_border(next(ck)))
				ck = next(prev_on_border(next(ck)));
			else
				ck = next(inv(next(ck)));
		} while (ck != ci);
*/
		if (!trace_strip)
			continue;
		// trace a triangle strip over intersected edges
		do {
			if (C::is_opposite_to_border(ck)) {
				std::cerr << "ray trace leaves convex hull" << std::endl;
				return;
			}
			ck = C::inv(ck);
			unsigned int vk = C::vi_of_ci(ck);
			if (vk == vj)
				return;

			const point_type& pk = G::p_of_vi(vk);
			bool out_ij = geometry_type::is_outside(pk, pi, pj);
			bool out_ji = geometry_type::is_outside(pk, pj, pi);
			if (!out_ij && !out_ji) {
				trace_hits.push_back(trace_hit(INCIDENT_VERTEX, vk));
				break;
			}
			if (out_ij)
				ck = C::prev(ck);
			else
				ck = C::next(ck);
			trace_hits.push_back(trace_hit(INCIDENT_EDGE, ck));
		} while (true);
	} while (true);
}

/// compute the circum center of a triangle
template <class G, class C>
typename triangle_mesh<G,C>::point_type triangle_mesh<G,C>::compute_circum_center(unsigned int ci) const
{
	return geometry_type::compute_circum_center(G::p_of_vi(C::vi_of_ci(ci)), G::p_of_vi(C::vi_of_ci(C::next(ci))), G::p_of_vi(C::vi_of_ci(C::prev(ci))));
}

/// flip the edges that intersect segment between vi and vj such that (vi,vj) becomes an edge of the triangulation
template <class G, class C>
void triangle_mesh<G,C>::flip_edges_to_insert_segment(unsigned int vi, unsigned int vj, std::vector<unsigned int>& cis)
{
	unsigned int  k;
	std::cout << "flip_edges_valid(" << vi << ", " << vj << ")";
/*	for (k=0; k<cis.size(); ++k) {
		if (cis[k] == -1)
			continue;
		std::cout << "(" << vi_of_ci(next(cis[k])) << "," << vi_of_ci(prev(cis[k])) << ") ";
	}*/
	std::cout << std::endl;

	const point_type& pi = G::p_of_vi(vi);
	const point_type& pj = G::p_of_vi(vj);
	unsigned int i = 0;
	unsigned int j = 0;
	while (i < cis.size()) {
		std::cout << "i=" << i << ", j=" << j << ": ";
		for (k=0; k<cis.size(); ++k) {
			if (cis[k] == -1)
				continue;
			std::cout << "(" << C::vi_of_ci(C::next(cis[k])) << "," << C::vi_of_ci(C::prev(cis[k])) << ") ";
		}
		std::cout << std::endl;
		unsigned int c0 = cis[j];
		unsigned int c1 = C::next(c0);
		unsigned int c2 = C::next(c1);
		unsigned int c3 = C::inv(c0);
		unsigned int c4 = C::next(c3);
		unsigned int v0 = C::vi_of_ci(c0);
		unsigned int v1 = C::vi_of_ci(c1);
		unsigned int v2 = C::vi_of_ci(c2);
		unsigned int v3 = C::vi_of_ci(c3);
		const point_type& p0 = G::p_of_vi(v0);
		const point_type& p1 = G::p_of_vi(v1);
		const point_type& p2 = G::p_of_vi(v2);
		const point_type& p3 = G::p_of_vi(v3);
		bool p0_and_p1_same_side =
					v0 == vi ||
					geometry_type::is_outside(p0,pi,pj) &&
					geometry_type::is_outside(p1,pi,pj) ||
					geometry_type::is_inside(p0,pi,pj) &&
					geometry_type::is_inside(p1,pi,pj);
		bool flipable = is_edge_flip_valid(c0);
		bool push = false;
		bool do_flip = false;
		if (p0_and_p1_same_side) {
			bool p2_and_p3_same_side =
						v3 == vj ||
						geometry_type::is_outside(p2,pi,pj) &&
						geometry_type::is_outside(p3,pi,pj) ||
						geometry_type::is_inside(p2,pi,pj) &&
						geometry_type::is_inside(p3,pi,pj);
			if (flipable) {
				if (p2_and_p3_same_side) {
					// case 1
					do_flip = true;
				}
				else {
					// case 2
					do_flip = true;
				}
			}
			else {
				if (p2_and_p3_same_side) {
					// case 3
					push = true;
				}
				else {
					// case 4
					push = true;
				}
			}
		}
		else {
			bool p1_and_p3_same_side =
						v3 == vj ||
						geometry_type::is_outside(p1,pi,pj) &&
						geometry_type::is_outside(p3,pi,pj) ||
						geometry_type::is_inside(p1,pi,pj) &&
						geometry_type::is_inside(p3,pi,pj);
			if (flipable) {
				if (p1_and_p3_same_side) {
					// case 5
					do_flip = true;
				}
				else {
					// case 6
					do_flip = true;
				}
			}
			else {
				if (p1_and_p3_same_side) {
					// case 7
					push = true;
				}
				else {
					// case 8
					push = true;
				}
			}
		}
		if (push) {
			std::cout << "pushed edge(" << v1 << "," << v2 << ")\n";
			j = j+1;
			if (j == cis.size()) {
				std::cerr << "ups: reached last edge" << std::endl;
				return;
			}
		}
		else {
			std::cout << "flipped edge(" << v1 << "," << v2 << ")\n";
			C::flip_edge(c0);
			for (unsigned int k=(j==0?0:j-1); k<(j+1==cis.size()?j+1:j+2); ++k) {
				if (cis[k] == c1)
					cis[k] = c3;
				else if (cis[k] == c4)
					cis[k] = c0;
				else if (cis[k] == c0)
					cis[k] = c1;
				else if (cis[k] == c3)
					cis[k] = c4;
			}
			if (j == i) {
				++i;
				++j;
				std::cout << "step" << std::endl;
			}
			else {
				--j;
				std::cout << "pop" << std::endl;
			}
		}
	}
/*	bool changed;
	unsigned int n = (unsigned int) cis.size();
	unsigned int  k;
	std::cout << "flip_edges_valid() called with\n";
	for (k=0; k<cis.size(); ++k) {
		if (cis[k] == -1)
			continue;
		std::cout << "(" << vi_of_ci(next(cis[k])) << "," << vi_of_ci(prev(cis[k])) << ") ";
	}
	const point_type& pi = p_of_vi(vi);
	const point_type& pj = p_of_vi(vj);
	std::cout << std::endl;
	do {
		changed = false;
		for (unsigned int i=0; i<cis.size(); ++i) {
			if (cis[i] == -1)
				continue;
			if (!is_edge_flip_valid(cis[i])) {
				std::cerr << "invalid edge (" << vi_of_ci(next(cis[i])) << "," << vi_of_ci(prev(cis[i])) << ") " << std::endl;
				continue;
			}
			unsigned int c0 = cis[i];
			unsigned int c1 = next(c0);
			unsigned int c2 = next(c1);
			unsigned int c3 = inv(c0);
			unsigned int c4 = next(c3);
			unsigned int c5 = next(c4);

			unsigned int v0 = vi_of_ci(c0);
			unsigned int v3 = vi_of_ci(c3);
			const point_type& p0 = p_of_vi(v0);
			const point_type& p3 = p_of_vi(v3);
			if (!(v0 == vi || v3 == vj ||
				   v0 == vj || v3 == vi ||
				   geometry_type::is_outside(p0,pi,pj) && geometry_type::is_outside(p3,pi,pj) ||
				   geometry_type::is_inside(p0,pi,pj) && geometry_type::is_inside(p3,pi,pj) ) ) {
				std::cerr << "unsolving constraint edge (" << vi_of_ci(next(cis[i])) << "," << vi_of_ci(prev(cis[i])) << ") " << std::endl;
				continue;
			}

			std::cout << "flipped edge(" << vi_of_ci(c1) << "," << vi_of_ci(c2) << ")\n";
			flip_edge(cis[i]);
			cis[i] = -1;
			--n;
			for (unsigned int j=0; j<cis.size(); ++j) {
				if (cis[j] == c1)
					cis[j] = c3;
				if (cis[j] == c4)
					cis[j] = c0;
			}
			for (k=0; k<cis.size(); ++k) {
				if (cis[k] == -1)
					continue;
				std::cout << "(" << vi_of_ci(next(cis[k])) << "," << vi_of_ci(prev(cis[k])) << ") ";
			}
			std::cout << std::endl;
			changed = true;
		}
	} while (changed);
	if (n > 0)
		std::cerr << "could not flip all edges!!!" << std::endl;
	else
		std::cerr << "all edges flipped" << std::endl;*/
}

///
template <class G, class C>
bool triangle_mesh<G,C>::is_edge_flip_valid(unsigned int c0) const
{
	if (C::is_opposite_to_border(c0))
		return false;
	unsigned int c1 = C::next(c0);
	unsigned int c2 = C::next(c1);
	unsigned int c3 = C::inv(c0);
	const point_type& p0 = G::p_of_vi(C::vi_of_ci(c0));
	const point_type& p1 = G::p_of_vi(C::vi_of_ci(c1));
	const point_type& p2 = G::p_of_vi(C::vi_of_ci(c2));
	const point_type& p3 = G::p_of_vi(C::vi_of_ci(c3));
	if (!geometry_type::is_outside(p1,p0,p3))
		return false;
	if (!geometry_type::is_outside(p2,p3,p0))
		return false;
	return true;
}
