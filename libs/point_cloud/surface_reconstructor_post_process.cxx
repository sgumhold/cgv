
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <set>
#include "surface_reconstructor.h"
#include <cgv/utils/progression.h>


bool surface_reconstructor::is_manifold_hole(std::vector<unsigned int>& V) const
{
	std::set<unsigned int> S;
	for (unsigned int i=0; i<V.size(); ++i)
		S.insert(V[i]);
	return S.size() == V.size();
}

bool surface_reconstructor::is_valid_ear(
	unsigned int vi, unsigned int vj, unsigned int vk) const
{
	Crd ljk = length(pc->pnt(vj) - pc->pnt(vk));
	if (ljk > valid_length_scale*get_reference_length(vi))
		return false;
//	if (ljk > valid_length_scale*get_reference_length(vk))
//		return false;
	if (!perform_intersection_tests)
		return true;
	if (can_add_triangle_manifold(vi,vj,vk))
		return true;
	else {
		if (debug_hole)
			std::cout << "found self intersection of triangle " << vi << "," << vj << "," << vk << std::endl;
		return false;
	}
}

surface_reconstructor::Crd surface_reconstructor::compute_triangle_area(
	unsigned int vi, unsigned int vj, unsigned int vk) const
{
	const Pnt& pi = pc->pnt(vi);
	return length(cross(pc->pnt(vj)-pi, pc->pnt(vk)-pi));
}

bool surface_reconstructor::find_min_area_hole_tesselation(
	const std::vector<unsigned int>& V, std::vector<unsigned int>& tess, Crd& area) const
{
	std::vector<unsigned int> opt_tess;
	unsigned int i, n = (unsigned int) V.size();
	for (i=2; i<n; ++i) {

		// show progression
		if (n > 13)
			std::cout << n << ":" << i << " started" << std::endl;

		unsigned int vi = V[i];
		unsigned int vj = V[(i+1)%n];
		unsigned int vk = V[(i+n-1)%n];
		if (is_valid_ear(vi,vj,vk)) {

			// start tesselation with one ear
			std::vector<unsigned int> new_tess;
			new_tess.push_back(i);

			// compute area of ear
			Crd new_area = compute_triangle_area(vi,vj,vk);
			bool success = true;
			if (n > 3) {
				// prepare recursion
				std::vector<unsigned int> sub_V = V;

				// cut away the ear
				sub_V.erase(sub_V.begin()+i);

				// recursion
				Crd sub_area;
				success = find_min_area_hole_tesselation(sub_V, new_tess, sub_area);
				if (success)
					new_area += sub_area;
			}
			if (success) {
				if (debug_hole)
					std::cout << n << ":" << i << " -> ";
				if (opt_tess.empty() || new_area < area) {
					if (debug_hole) {
						if (opt_tess.empty())
							std::cout << "first area:  ";
						else
							std::cout << "better area: ";
					}

					area = new_area;
					opt_tess = new_tess;
				}
				else {
					if (debug_hole)
						std::cout << "worse area:  ";
				}
				if (debug_hole)
					std::cout << new_area << std::endl;
			}
		}
		else
			if (debug_hole)
				std::cout << n << ":" << i << " ear " << vi << "," << vj << "," << vk << " blocked" << std::endl;
	}
//	if (n > 15)
//		std::cout << n << ":" << i << " finished" << std::endl;
	if (opt_tess.empty())
		return false;
	for (i=0; i<opt_tess.size(); ++i)
		tess.push_back(opt_tess[i]);
	return true;
}

struct pred
{
	bool operator() (const std::pair<float,unsigned int>& l, const std::pair<float,unsigned int>& r) const
	{
		return l.first < r.first;
	}
};

bool surface_reconstructor::greedy_find_min_area_hole_tesselation(
	const std::vector<unsigned int>& V, std::vector<unsigned int>& tess, Crd& area) const
{
	std::vector<unsigned int> opt_tess;
	unsigned int i, n = (unsigned int) V.size();

	// sort ears by length of missing edge
	std::vector<std::pair<float,unsigned int> > ext_V;
	ext_V.resize(n);
	for (i=0; i<n; ++i) {
		ext_V[i].first = sqr_length(pc->pnt(V[(i+1)%n]) - pc->pnt(V[(i+n-1)%n]));
		ext_V[i].second = i;
	}
	std::sort(ext_V.begin(),ext_V.end(),pred());
	
	// iterate in sorted order
	unsigned int j,nr_recursions = 0;
	for (j=0; j<n-2; ++j) {
		unsigned int i = ext_V[j].second;
		unsigned int vi = V[i];
		unsigned int vj = V[(i+1)%n];
		unsigned int vk = V[(i+n-1)%n];
		if (is_valid_ear(vi,vj,vk)) {

			// start tesselation with one ear
			std::vector<unsigned int> new_tess;
			new_tess.push_back(i);

			// compute area of ear
			Crd new_area = compute_triangle_area(vi,vj,vk);
			bool success = true;
			if (n > 3) {
				// prepare recursion
				std::vector<unsigned int> sub_V = V;

				// cut away the ear
				sub_V.erase(sub_V.begin()+i);

				// recursion
				Crd sub_area;
				success = greedy_find_min_area_hole_tesselation(sub_V, new_tess, sub_area);
				if (success)
					new_area += sub_area;
			}
			if (success) {
				if (debug_hole)
					std::cout << n << ":" << i << " -> ";
				if (opt_tess.empty() || new_area < area) {
					if (debug_hole) {
						if (opt_tess.empty())
							std::cout << "first area:  ";
						else
							std::cout << "better area: ";
					}

					area = new_area;
					opt_tess = new_tess;
				}
				else {
					if (debug_hole)
						std::cout << "worse area:  ";
				}
				if (debug_hole)
					std::cout << new_area << std::endl;
				break;
			}
			else
				if (n >= max_nr_exhaustive_search || ++nr_recursions > 4)
					break;
		}
		else
			if (debug_hole)
				std::cout << n << ":" << i << " ear " << vi << "," << vj << "," << vk << " blocked" << std::endl;
	}
//	if (n > 15)
//		std::cout << n << ":" << i << " finished" << std::endl;
	if (opt_tess.empty())
		return false;
	for (i=0; i<opt_tess.size(); ++i)
		tess.push_back(opt_tess[i]);
	return true;
}


void surface_reconstructor::close_hole_with_tesselation(
	const std::vector<unsigned int>& _V, const std::vector<unsigned int>& tess, std::vector<unsigned int>& T)
{
	std::vector<unsigned int> V = _V;
	unsigned int j, m = (unsigned int) tess.size();
	for (j=0; j<m; ++j) {
		unsigned int n = (unsigned int) V.size();
		unsigned int i = tess[j];
		unsigned int vi = V[i];
		unsigned int vj = V[(i+1)%n];
		unsigned int vk = V[(i+n-1)%n];
		add_hole_triangle(vi,vj,vk,T);
		V.erase(V.begin()+i);
	}
}

void surface_reconstructor::resolve_non_manifold_vertex(unsigned int vi, std::vector<unsigned int>& T)
{
	// reference neighborhood Ni of vi
	const std::vector<Idx> &Ni = ng->at(vi);
	unsigned int n = (int) Ni.size();
	// find first non face corner
	unsigned int j0;
	for (j0=0; j0<n; ++j0)
		if (!is_face_corner(vi,j0))
			break;
	// if only face corner skip this vertex
	if (j0 == n) {
		std::cout << "resolve_non_manifold_vertex called for one ring vertex" << std::endl;
		return;
	}
	bool last_is_face_corner = false;
	std::vector<unsigned int> lim;
	unsigned int j;
	for (j = (j0+1)%n; true; j = (j+1)%n) {
//		std::cout << j << "(" << n << ")" << (last_is_face_corner?"F":"_")
//				    << (is_face_corner(vi,j)?"F":"_") << std::endl;
		if (is_face_corner(vi,j)) {
			if (!last_is_face_corner)
				lim.push_back(j);
			last_is_face_corner = true;
		}
		else {
			if (last_is_face_corner)
				lim.push_back(j);
			last_is_face_corner = false;
		}
		if (j == j0)
			break;
	}
	if ((lim.size()&1) != 0) {
		std::cout << "found odd number " << lim.size() << " of limits for vertex " << vi << std::endl;
		return;
	}
	unsigned int x = 0;
//	std::cout << lim.size()/2 << " fans at " << vi << std::endl;
	Crd quality = compute_fan_quality(vi,lim[0],lim[1]);
	for (j=2; j<lim.size(); j+=2) {
		Crd new_quality = compute_fan_quality(vi,lim[j],lim[j+1]);
		if (new_quality > quality) {
			quality = new_quality;
			x = j;
		}
	}
//	std::cout << "chose " << x << "-th fan" << std::endl;
	for (j=0; j<lim.size(); j+=2) if (j != x) {
//		std::cout << "fan " << j << ": " << lim[0] << ":" 
//			<< Ni[lim[j]] << " -> " << lim[j+1] << ":" << Ni[lim[j+1]] << std::endl;
		for (unsigned int y = lim[j]; y != lim[j+1]; y = (y+1)%n) {
			remove_triangle(vi, Ni[y], Ni[(y+1)%n],T);
		}
	}
}

surface_reconstructor::Crd surface_reconstructor::compute_fan_quality(unsigned int vi, unsigned int j, unsigned int k) const
{
	// reference neighborhood Ni of vi
	const std::vector<Idx> &Ni = ng->at(vi);
	unsigned int n = (int) Ni.size();

	unsigned int vj = Ni[j];
	unsigned int vk = Ni[k];
	const Pnt& pi = pc->pnt(vi);
	Crd l_max = 0;
	Crd a_sum = 0;
	unsigned int m = 0;
	for (unsigned int l = j; l != k; l = (l+1)%n) {
		++m;
		unsigned int vl = Ni[l];
		const Pnt& pl = pc->pnt(vl);
		l_max = std::max(length(pl-pi),l_max);
		Crd a = (Crd)compute_corner_angle(vi,l,(l+1)%n);
		a_sum += a;
	}
//	Crd q1 = a_sum/m/l_max;
	Crd q2 = a_sum/l_max;
//	printf("fan %2d,%2d: m=%2d, l=%4.2f, a=%4.2f ==> q1=%4.2f, q2=%4.2f\n",
//		vj,vk,m,l_max,a_sum,q1,q2);
	return q2;
}

void surface_reconstructor::remove_triangle(unsigned int vi, unsigned int vj, unsigned int vk, std::vector<unsigned int>& T)
{
//	std::cout << "remove triangle " << vi << "," << vj << "," << vk << std::endl;
//	std::cout << "count:" << std::endl;
	count_triangle(vi,vj,vk,false);
//	std::cout << "mark:" << std::endl;
	mark_triangle(vi,vj,vk,false);
//	std::cout << "find:" << std::endl;
	for (unsigned int ti=0; ti < T.size(); ti +=3) {
		bool found = true;
		for (unsigned int c=0; c<3; ++c) {
			if (!(T[ti+c] == vi || T[ti+c] == vj || T[ti+c] == vk)) {
				found = false;
				break;
			}
		}
		if (found) {
//			std::cout << "erase:" << std::endl;
			T.erase(T.begin()+ti, T.begin()+ti+3);
			return;
		}
	}
//	std::cout << "could not find to be removed triangle" << std::endl;
}

unsigned int surface_reconstructor::resolve_non_manifold_vertices(std::vector<unsigned int>& T)
{
	unsigned int vi, n = (unsigned int) pc->get_nr_points();
	unsigned int iter = 0;
	for (vi = 0; vi < n; ++vi) {
		if (get_vertex_type(vi) == VT_NM_BORDER) {
			++iter;
			resolve_non_manifold_vertex(vi, T);
		}
	}
	return iter;
}

unsigned int surface_reconstructor::resolve_non_manifold_holes(std::vector<unsigned int>& T)
{
	unsigned int vi = 0;
	int j = -1;
	unsigned int iter = 0;
	clear_flag();
	do {
		std::vector<unsigned int> V;
		HoleType ht = find_next_hole(vi,j,V);
		if (ht == HT_NONE)
			break;
		if (ht == HT_NM) {
//			std::cout << "found nm hole of length " << V.size() << std::endl;
			std::sort(V.begin(),V.end());
			unsigned int i=0;
			while (i < V.size()-1) {
				if (V[i] == V[i+1]) {
					resolve_non_manifold_vertex(V[i],T);
//					std::cout << "resolve nm vertex " << V[i] << std::endl;
					++iter;
					do {
						++i;
					} while (i < V.size()-1 && V[i] == V[i+1]);
				}
				else
					++i;
			}
		}
		else {
			mark_hole(V);
		}
	} while (true);
	clear_flag();
	return iter;
}


bool surface_reconstructor::step_hole(unsigned int& vi, int& j) const
{
	unsigned int vj = ng->at(vi)[j];
	int i  = ng->find(vj,vi);
	if (i == -1)
		return false;
	unsigned int nj = (int)ng->at(vj).size();
	if (use_orientation || is_face_corner(vj,i)) {
		if (!is_face_corner(vj,i))
			return false;
		i = (i+nj-1)%nj;
		if (is_face_corner(vj,i))
			return false;
		do {
			unsigned int k = (i+nj-1)%nj;
			if (is_face_corner(vj,k))
				break;
			i = k;
		} while (true);
	}
	else {
//		std::cout << "orientation change from " << vi << " to " << vj << std::endl;
		unsigned int k = (i+nj-1)%nj;
		if (!is_face_corner(vj,k)) {
//			std::cout << "no face corner at " << vj << " at " << ng->at(vj)[k] << std::endl;
			return false;
		}
		do {
			i = (i+1)%nj;
		} while (!is_face_corner(vj,i));
//		std::cout << "found face corner at " << vj << " to " << ng->at(vj)[i] << std::endl;
	}
	vi = vj;
	j  = i;
	return true;
}

///
void surface_reconstructor::mark_hole(const std::vector<unsigned int>& V, bool flag)
{
	if (V.size() < 2)
		return;
	unsigned int vi = V[0];
	int j  = ng->find(vi,V[1]);
	for (unsigned int k=0; k<V.size(); ++k) {
		mark(vi,j);
		step_hole(vi,j);
	}
}

/// find the next hole
surface_reconstructor::HoleType surface_reconstructor::trace_hole(unsigned int vi, int j, std::vector<unsigned int>& V)
{
	unsigned int vi_start = vi;
	HoleType ht = HT_NONE;
	std::vector<unsigned int> J;
	V.clear();
	J.clear();
	while (!is_marked(vi,j)) {
		V.push_back(vi);
		J.push_back(j);
		mark(vi,j);
		if (!step_hole(vi,j)) {
			ht = HT_INCONSISTENT;
			break;
		}
	}
	// unmark hole
	for (unsigned k=0; k<V.size(); ++k)
		mark(V[k],J[k],false);
	
	if (ht == HT_NONE) {
		if (vi == vi_start && std::find(V.begin()+1,V.end(),vi) == V.end())
			ht = HT_MANIFOLD;
		else
			ht = HT_NM;
	}
	return ht;
}

/// find the next hole
bool surface_reconstructor::find_next_hole_start(unsigned int& vi, int& j) const
{
	if (vi == -1)
		vi = 0;
	unsigned int n = (int) pc->get_nr_points();
	if (vi >= n)
		return false;

	do {
		// reference neighborhood Ni of vi
		const std::vector<Idx> &Ni = ng->at(vi);
		int ni = (int) Ni.size();
		int last_j = (j+ni)%ni;
		for (++j; j < ni; ++j) {
			if (is_face_corner(vi,last_j))
				if (!is_face_corner(vi,j))
					return true;
			last_j = j;
		}
		++vi;
		j = -1;
	} while (vi < n);
	return false;
}


/// find the next hole
surface_reconstructor::HoleType surface_reconstructor::find_next_hole(unsigned int& vi, int& j, std::vector<unsigned int>& V)
{
	while (find_next_hole_start(vi,j)) {
		if (!is_marked(vi,j)) {
			HoleType ht = trace_hole(vi,j,V);
			if (ht != HT_NONE) {
				if (ht == HT_MANIFOLD) {
					if (!is_manifold_hole(V))
						ht = HT_NM;
				}
				return ht;
			}
		}
	}
	return HT_NONE;
}


surface_reconstructor::Crd surface_reconstructor::vol(unsigned int vi, const unsigned int* T) const
{
	const Pnt& p0 = pc->pnt(T[0]);
	return dot(pc->pnt(vi) - p0, cross(pc->pnt(T[1]) - p0, pc->pnt(T[2]) - p0));
}

surface_reconstructor::Crd surface_reconstructor::vol(unsigned int vi, unsigned int vj, unsigned int vk, unsigned int vl) const
{
	unsigned int T[3] = {vj,vk,vl};
	return vol(vi,T);
}

bool surface_reconstructor::side(unsigned int vi, const unsigned int* T) const
{
	return vol(vi,T) > 0;
}

bool surface_reconstructor::side(unsigned int vi, unsigned int vj, unsigned int vk, unsigned int vl) const
{
	unsigned int T[3] = {vj,vk,vl};
	return side(vi,T);
}

bool surface_reconstructor::reduced_tgl_tgl_intersection_test(
	unsigned int vi, unsigned int vj1, unsigned int vk1, unsigned int vj2, unsigned int vk2) const
{
	bool result = side(vj2,vi,vj1,vk1) != side(vk2,vi,vj1,vk1) &&
					  side(vj1,vi,vj2,vk2) != side(vk1,vi,vj2,vk2);
	if (result) {
		Crd Vj1 = vol(vj1,vi,vj2,vk2);
		Crd Vk1 = vol(vk1,vi,vj2,vk2);
		Crd lambda_1 = Vj1/(Vj1-Vk1);
		if (lambda_1 >= 0 && lambda_1 <= 1) {
			Crd Vj2 = vol(vj2,vi,vj1,vk1);
			Crd Vk2 = vol(vk2,vi,vj1,vk1);
			Crd lambda_2 = Vj2/(Vj2-Vk2);
			if (lambda_2 >= 0 && lambda_2 <= 1) {
				Pnt p1 = pc->pnt(vj1) + lambda_1 * (pc->pnt(vk1) - pc->pnt(vj1));
				Pnt p2 = pc->pnt(vj2) + lambda_2 * (pc->pnt(vk2) - pc->pnt(vj2));
				const Pnt& pi = pc->pnt(vi);
				if (dot(p1-pi, p2-pi) > 0) {
					if (debug_intersection_tests) {
						std::cout << "reduced tgl tgl test hit with lambda_1 = " << 
							lambda_1 << " and lambda_2 = " << lambda_2 << 
							", dot = " << dot(p1-pi, p2-pi) << std::endl;
					}
					return true;
				}
			}
		}

	}
	return false;
}

bool surface_reconstructor::tgl_tgl_intersection_test(const unsigned int* t1, const unsigned int* t2) const
{
	unsigned int i, j, k, n = 0;
	for (i=0; i < 3; ++i) {
		if (t1[i] == t2[0]) {
			++n;
			j = i;
			k = 0;
		}
		else if (t1[i] == t2[1]) {
			++n;
			j = i;
			k = 1;
		}
		else if (t1[i] == t2[2]) {
			++n;
			j = i;
			k = 2;
		}	
	}
	if (n >= 2)
		return false;
	if (n == 1)
		return reduced_tgl_tgl_intersection_test(t1[j],t1[(j+1)%3],t1[(j+2)%3],t2[(k+1)%3],t2[(k+2)%3]);

	bool s1[3] = {
		side(t1[0],t2),
		side(t1[1],t2),
		side(t1[2],t2)
	};
	if (s1[0] == s1[1] && s1[0] == s1[2])
		return false;

	bool s2[3] = {
		side(t2[0],t1),
		side(t2[1],t1),
		side(t2[2],t1)
	};
	if (s2[0] == s2[1] && s2[0] == s2[2])
		return false;

	unsigned int o1 = 0;
	if (s1[0] == s1[1])
		o1 = 2;
	else if (s1[0] == s1[2])
		o1 = 1;

	unsigned int o2 = 0;
	if (s2[0] == s2[1])
		o2 = 2;
	else if (s2[0] == s2[2])
		o2 = 1;

	unsigned int T1[3] = { t1[o1], t1[(o1+1)%3], t1[(o1+2)%3] };
	unsigned int T2[3] = { t2[o2], t2[(o2+1)%3], t2[(o2+2)%3] };

	Crd Vi2 = vol(T1[0],T2);
	Crd lambda_j1 = Vi2/(Vi2-vol(T1[1],T2));
	Crd lambda_k1 = Vi2/(Vi2-vol(T1[2],T2));

	const Pnt& p1_i = pc->pnt(T1[0]);
	Pnt p1_ij = p1_i + lambda_j1 * (pc->pnt(T1[1]) - p1_i);
	Pnt p1_ik = p1_i + lambda_k1 * (pc->pnt(T1[2]) - p1_i);

	Crd Vi1 = vol(T2[0],T1);
	Crd lambda_j2 = Vi1/(Vi1-vol(T2[1],T1));
	Crd lambda_k2 = Vi1/(Vi1-vol(T2[2],T1));

	const Pnt& p2_i = pc->pnt(T2[0]);
	Pnt p2_ij = p2_i + lambda_j2 * (pc->pnt(T2[1]) - p2_i);
	Pnt p2_ik = p2_i + lambda_k2 * (pc->pnt(T2[2]) - p2_i);

	bool result = (dot(p2_ij - p1_ij, p2_ik - p1_ij) < 0) ||
					  (dot(p2_ij - p1_ik, p2_ik - p1_ik) < 0) ||
					  (dot(p1_ij - p2_ij, p1_ik - p2_ij) < 0) ||
					  (dot(p1_ij - p2_ik, p1_ik - p2_ik) < 0);
	if (result) {
		if (debug_intersection_tests) {
			std::cout <<  "side1=(" 
						 << (s1[0]?"t":"f") 
						 << (s1[1]?"t":"f") 
						 << (s1[2]?"t":"f") 
						 << "), side2=(" 
						 << (s2[0]?"t":"f") 
						 << (s2[1]?"t":"f") 
						 << (s2[2]?"t":"f") 
						 << ")" << std::endl;
			std::cout <<  "o1=" << o1 << ", o2 = " << o2 << std::endl;				
			std::cout <<  "lj1=" << lambda_j1 << ", lk1 = " << lambda_k1 
						 << ", lj2=" << lambda_j2 << ", lk2 = " << lambda_k2 << std::endl;
		}
		return true;
	}
	return false;
}

surface_reconstructor::Crd surface_reconstructor::compute_normal_cosine(
	const Pnt& pi, const Pnt& pl, const Pnt& pj, const Pnt& pk) const
{
	return dot(normalize(cross(pl-pi,pj-pi)),normalize(cross(pj-pi,pk-pi)));
}

surface_reconstructor::Crd surface_reconstructor::compute_hole_triangle_quality(
	unsigned int vi, unsigned int vj, unsigned int vk) const
{
	const Pnt& pi = pc->pnt(vi);
	const Pnt& pj = pc->pnt(vj);
	const Pnt& pk = pc->pnt(vk);
	Crd c = dot(hole_nml, normalize(cross(pj-pi,pk-pi)));
	if (c <= 0)
		return 0;
	Crd l = length(pj-pk);
	if (l > 2.5f*get_reference_length(vi))
		return 0;
	if (!can_create_triangle_without_self_intersections(vi,vj,vk))
		return 0;
	return c/l;
}

/// step wise closing a hole
void surface_reconstructor::build_hole_closing_queue()
{
	hole_queue.clear();
	compute_hole_normal();
	unsigned int i, n = (unsigned int) hole.size();
	for (i=0; i<n; ++i) {
		unsigned int vi = hole[i];
		unsigned int vj = hole[(i+1)%n];
		unsigned int vk = hole[(i+n-1)%n];
		Crd q = compute_hole_triangle_quality(vi,vj,vk);
		if (q == 0)
			continue;
		hole_queue.insert(ear(i,q));
	}
}

void surface_reconstructor::compute_hole_normal()
{
	unsigned int i, n = (unsigned int) hole.size();
	hole_nml = Nml(0,0,0);
	for (i=0; i<n; ++i) {
		unsigned int vi = hole[i];
		unsigned int vj = hole[(i+1)%n];
		const Pnt& pi = pc->pnt(vi);
		const Pnt& pj = pc->pnt(vj);
		hole_nml += cross(pi,pj);
	}
	hole_nml = normalize(hole_nml);
}

void surface_reconstructor::add_hole_triangle(unsigned int vi, unsigned int vj, unsigned int vk, std::vector<unsigned int>& T)
{
//	mark_triangle(vi,vj,vk);
//	count_triangle(vi,vj,vk);
	T.push_back(vi);
	T.push_back(vj);
	T.push_back(vk);
//	std::cout << "adding triangle " << vi << "," << vj << "," << vk << std::endl;
}
///
void surface_reconstructor::step_hole_closing(std::vector<unsigned int>& T)
{
	if (hole.size() < 3) {
		std::cout << "ups hole of length < 3!!" << std::endl;
		exit(0);
	}
	if (hole.size() == 3) {
		unsigned int vi = hole[0];
		unsigned int vj = hole[1];
		unsigned int vk = hole[2];
		if (can_create_triangle_without_self_intersections(vi,vj,vk))
			add_hole_triangle(vi,vj,vk,T);
		hole.clear();
	}
	else if (hole.size() == 4) {
		compute_hole_normal();
		std::cout << "hole size 4" << std::endl;
		unsigned int vi = hole[0];
		unsigned int vj = hole[1];
		unsigned int vk = hole[2];
		unsigned int vl = hole[3];
		Crd q[4] = {
			compute_hole_triangle_quality(vi,vj,vl),
			compute_hole_triangle_quality(vj,vk,vi),
			compute_hole_triangle_quality(vk,vl,vj),
			compute_hole_triangle_quality(vl,vi,vk)
		};
		bool possibility_1 = 
				 q[0] > 0 && q[2] > 0 &&
				 can_create_triangle_without_self_intersections(vi,vj,vl) &&
				 can_create_triangle_without_self_intersections(vk,vl,vj);
		bool possibility_2 = 
				 q[1] > 0 && q[3] > 0 &&
				 can_create_triangle_without_self_intersections(vj,vk,vi) &&
				 can_create_triangle_without_self_intersections(vl,vi,vk);
		if (possibility_1 && possibility_2) {
			if (q[0]+q[2] < q[1]+q[3])
				possibility_1 = false;
			else
				possibility_2 = false;
		}
		if (possibility_1) {
			add_hole_triangle(vi,vj,vl,T);
			add_hole_triangle(vk,vl,vj,T);
		}
		if (possibility_2) {
			add_hole_triangle(vj,vk,vi,T);
			add_hole_triangle(vl,vi,vk,T);
		}
		hole.clear();
	}
	else {
		build_hole_closing_queue();
		if (!hole_queue.empty()) {
			unsigned int i = hole_queue[hole_queue.top()].i;
			unsigned int n  = (unsigned int) hole.size();
			unsigned int vi = hole[i];
			unsigned int vj = hole[(i+1)%n];
			unsigned int vk = hole[(i+n-1)%n];
			add_hole_triangle(vi,vj,vk,T);
			hole.erase(hole.begin()+i);
		}
	}
}

/// close a hole by ear cutting
bool surface_reconstructor::close_hole(std::vector<unsigned int>& V)
{
	unsigned int i, n = (unsigned int) V.size();
	std::vector<bool> is_convex;

	std::vector<Crd> quality;
	is_convex.resize(n);

	for (i=0; i<n; ++i) {
		
	}
	return false;
}

void surface_reconstructor::close_holes(std::vector<unsigned int>& T)
{
	unsigned int vi = 0;
	int j = -1;
//	double tmp = valid_length_scale;
//	valid_length_scale *= 2.5;
	unsigned int nr_nm = 0, nr_success = 0, nr_failed = 0, nr_intersecting_holes = 0;
	clear_flag();
	do {
		std::vector<unsigned int> V;
		HoleType ht = find_next_hole(vi,j,V);
		if (ht == HT_NONE)
			break;
		if (ht == HT_NM) {
			std::cout << "found nm hole of length " << V.size() << std::endl;
			++nr_nm;
		}
		else {
			Crd area;
			std::vector<unsigned int> tess;
			if (V.size() > max_nr_exhaustive_search) {
				bool test = greedy_find_min_area_hole_tesselation(V,tess,area);
				if (!test && perform_intersection_tests && allow_intersections_in_holes) {
					perform_intersection_tests = false;
					test = greedy_find_min_area_hole_tesselation(V,tess,area);
					if (test)
						++nr_intersecting_holes;
					perform_intersection_tests = true;
				}
				if (test) {
					close_hole_with_tesselation(V,tess,T);
					++nr_success;
				}	
				else
					++nr_failed;
			}
			else {
				if (V.size() > 10) {
					std::cout << "hole of size " << V.size() << std::endl;
				}
//				std::cout << "closing hole of size " << V.size() << std::endl;
				bool test = find_min_area_hole_tesselation(V,tess,area);
				if (!test && perform_intersection_tests && allow_intersections_in_holes) {
					perform_intersection_tests = false;
					test = find_min_area_hole_tesselation(V,tess,area);
					if (test)
						++nr_intersecting_holes;
					perform_intersection_tests = true;
				}
				if (test) {
					close_hole_with_tesselation(V,tess,T);
					++nr_success;
				}	
				else
					++nr_failed;
			}
			mark_hole(V);
		}
 	} while (true);
	std::cout << "closed " << nr_success << " holes " << nr_intersecting_holes << " with intersections and skipped " 
		<< nr_failed << " manifold and " << nr_nm << " non manifold holes" << std::endl;
	clear_flag();
//	valid_length_scale = tmp;
}


struct tgl
{
	unsigned int t[3];
	void init() { std::sort(t,t+3); }
	tgl(const unsigned int* T) { t[0] = T[0]; t[1] = T[1]; t[2] = T[2]; init(); }
	tgl() { t[0] = t[1] = t[2] = 0; init(); }
	tgl(unsigned int vi, unsigned int vj, unsigned int vk) { t[0] = vi; t[1] = vj; t[2] = vk; init(); }
	operator const unsigned int* () const { return t; };
	bool operator < (const tgl& T) const {
		return t[0] < T.t[0] || (t[0] == T.t[0] && (t[1] < T.t[1] ||
			(t[1] == T.t[1] && t[2] < T.t[2]) ) );
	}
};

std::ostream& operator << (std::ostream& os, const tgl& T)
{
	return os << T.t[0] << "," << T.t[1] << "," << T.t[2] << std::endl;
}

/// check if a triangle can be constructed without generating self intersections
bool surface_reconstructor::can_create_triangle_without_self_intersections(unsigned int vi,unsigned int vj,unsigned int vk) const
{
	neighbor_graph& NG = *ng;
	unsigned int t[3] = { vi,vj,vk };
	// collect potential neighbors
	const std::vector<Idx> &Ni = NG[vi];
	unsigned int ni = (unsigned int) Ni.size();
	const std::vector<Idx> &Nj = NG[vj];
	unsigned int nj = (unsigned int) Nj.size();
	const std::vector<Idx> &Nk = NG[vk];
	unsigned int nk = (unsigned int) Nk.size();
	std::set<Idx> VI;
	VI.insert(vi);
	VI.insert(vj);
	VI.insert(vk);
	unsigned int l;
	for (l=0;l<ni;++l)
		VI.insert(Ni[l]);
	for (l=0;l<nj;++l)
		VI.insert(Nj[l]);
	for (l=0;l<nk;++l)
		VI.insert(Nk[l]);

	// collect incident triangles
	std::set<tgl> T;
	for (std::set<Idx>::const_iterator iter = VI.begin(); iter != VI.end(); ++iter) {
		unsigned int vi = *iter;
		const std::vector<Idx> &Ni = NG[vi];
		unsigned int ni = (unsigned int) Ni.size();
		for (unsigned int j=0; j < ni; ++j) {
			if (is_face_corner(vi,j))
				T.insert(tgl(vi,Ni[j],Ni[(j+1)%ni]));
		}
	}
	for (std::set<tgl>::const_iterator jter = T.begin(); jter != T.end(); ++jter) {
		if (tgl_tgl_intersection_test(t, *jter)) {
			if (debug_intersection_tests)
				std::cout << "triangle " << vi << "," << vj << "," << vk 
							 << "  intersects " << *jter << std::endl;
			return false;
		}
	}
	return true;
}
