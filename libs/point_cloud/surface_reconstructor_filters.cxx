
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include "surface_reconstructor.h"
#include <cgv/utils/progression.h>

struct angle_sort_pred 
{
	const surface_reconstructor& sr;
	unsigned int vi;
	surface_reconstructor::Dir d0;

	angle_sort_pred(const surface_reconstructor& _sr) : sr(_sr) 
	{
	}
	void init(unsigned int vi, unsigned int vj)
	{
		this->vi  = vi;
		d0 = sr.compute_tangential_direction(vi,vj);
	}

	bool operator () (unsigned int vj, unsigned int vk) const
	{
		return sr.compute_tangential_corner_angle(vi,d0,vj) < sr.compute_tangential_corner_angle(vi,d0,vk);
	}
};

struct pred
{
	bool operator() (const std::pair<float,unsigned int>& l, const std::pair<float,unsigned int>& r) const
	{
		return l.first < r.first;
	}
};

void surface_reconstructor::sort_by_tangential_angle(unsigned int vi)
{
	std::vector<Idx>& Ni = ng->at(vi);
	Idx i, n = (Idx)Ni.size();
	// eliminate vertices in the origin of the tangential space
	for (i=0; i<n; ++i) {
		Vec dij = pc->pnt(Ni[i]) - pc->pnt(vi);
		Crd c = dot(dij,pc->nml(vi));
		if (sqrt(c*c / sqr_length(dij)) > 0.99) {
			Ni.erase(Ni.begin()+i);
			--n;
			--i;
		}
	}
	// choose the last neighbor as reference tangent vector, because it is the longest
	Dir d0 = compute_tangential_direction(vi,Ni.back());
	// pair indices with angles
	static std::vector<std::pair<float,unsigned int> > ext_Ni;
	ext_Ni.resize(n);
	ext_Ni[0] = std::pair<float,unsigned int>(0.0f, Ni.back());
	for (i=1; i<n; ++i)
		ext_Ni[i] = std::pair<float,unsigned int>((float)compute_tangential_corner_angle(vi,d0,Ni[i-1]),Ni[i-1]);
	// sort by angle
	static pred P;
	std::sort(ext_Ni.begin()+1,ext_Ni.end(),P);

	// eliminate neighbors with same angle that are further apart
	const Crd angle_eps = 0.01f;
	while (n > 1 && fabs(2*M_PI-ext_Ni.back().first) < angle_eps) {
		if (sqr_length(pc->pnt(ext_Ni.front().second) - pc->pnt(vi)) > 
			 sqr_length(pc->pnt(ext_Ni.back().second) - pc->pnt(vi)) ) 
		{
			int j = ng->find(vi,ext_Ni.front().second);
			if (j == -1) {
				std::cout << "could not find neighbor " << ext_Ni.front().second << " anymore" << std::endl;
				exit(0);
			}
			Ni.erase(Ni.begin()+j);
			ext_Ni.front().second = ext_Ni.back().second;
			ext_Ni.pop_back();
		}
		else {
			int k = ng->find(vi,ext_Ni.back().second);
			if (k == -1) {
				std::cout << "could not find neighbor " << ext_Ni.back().second << " anymore" << std::endl;
				exit(0);
			}
			Ni.erase(Ni.begin()+k);
			ext_Ni.pop_back();
		}
		--n;
	}
	for (i=0; i<n-1; ++i) {
		if (fabs(ext_Ni[i].first-ext_Ni[i+1].first) < angle_eps) {
			if (sqr_length(pc->pnt(ext_Ni[i].second) - pc->pnt(vi)) > 
				 sqr_length(pc->pnt(ext_Ni[i+1].second) - pc->pnt(vi)) ) 
			{
				int j = ng->find(vi,ext_Ni[i].second);
				if (j == -1) {
					std::cout << "could not find neighbor " << ext_Ni[i].second << " anymore" << std::endl;
					exit(0);
				}
				Ni.erase(Ni.begin()+j);
				ext_Ni.erase(ext_Ni.begin()+i);
			}
			else {
				int k = ng->find(vi,ext_Ni[i+1].second);
				if (k == -1) {
					std::cout << "could not find neighbor " << ext_Ni[i+1].second << " anymore" << std::endl;
					exit(0);
				}
				Ni.erase(Ni.begin()+k);
				ext_Ni.erase(ext_Ni.begin()+i+1);
			}
			--n;
			--i;
		}
	}
	if (n == 1) {
		std::cout << "removed all but one neighbor " << Ni[0] << " of vertex " << vi << std::endl;
		return;
	}
	directed_edge_info[vi].resize(n);
	// check whether there is an angle of PI or more
	bool is_border = false;
	float last_angle = ext_Ni.back().first-(float)(2*M_PI);
	Idx j;
	for (j=0; j<n; ++j) {
		float angle = ext_Ni[j].first;
		if (angle - last_angle > M_PI-0.1) {
			is_border = true;
			break;
		}
		last_angle = angle;
	}
	// copy sorted indices back with border between last and first neighbor
	for (i=0; i<n; ++i)
		Ni[i] = ext_Ni[(j+i)%n].second;
	// mark border edges as constraint
	if (is_border) {
		mark_as_border(vi);
		mark_as_constraint(vi,0);
		mark_as_constraint(vi,n-1);
	}

/*
	angle_sort_pred asp(*this);
	std::vector<Idx>& Ni = ng->at(vi);
	asp.init(vi, Ni[0]);
	std::sort(Ni.begin(), Ni.end(), asp);
*/
}

void surface_reconstructor::sort_by_tangential_angle()
{
	ensure_directed_edge_info();
	ensure_vertex_info();
	unsigned int n = (unsigned int) pc->get_nr_points();
	cgv::utils::progression prog("tangential sort          ", n, 10);
	for (unsigned int vi = 0; vi < n; ++vi) {
		// set reference length to median neighbor distance
		set_reference_length(vi,
			length(pc->pnt(vi) - 
			       pc->pnt(ng->at(vi).back())));
		sort_by_tangential_angle(vi);
		prog.step();
	}
}


bool surface_reconstructor::symmetrize_directed_edge(unsigned int vi, unsigned int j)
{
	if (debug_mode == DBG_SYMMETRIZE && debug_vi != -1 && debug_vi != vi)
		return false;
	neighbor_graph& NG = *ng;
	std::vector<Idx> &Ni = NG[vi];
	Cnt ni = (Cnt) Ni.size();

	Idx vj = Ni[j];
	std::vector<Idx> &Nj = NG[vj];
	Cnt nj = (Cnt) Nj.size();

	Idx k  = find_surrounding_corner(vj, vi);
	Idx vk = Nj[k];
	Idx vl = Nj[(k+1)%nj];

	if (debug_mode == DBG_SYMMETRIZE) {
		draw_colored_edge(vi,vj,4,0,0,1);
		if (is_face_corner(vj,k)) {
			draw_colored_edge(vj,vk,3,0,1,0);
			draw_colored_edge(vj,vl,3,0,1,0);
		}
		else{
			draw_colored_edge(vj,vk,3,1,1,0);
			draw_colored_edge(vj,vl,3,1,1,0);
		}

	}
	if (is_face_corner(vj,k)) {
		bool k_de = NG.is_directed_edge(vi,vk);
		bool l_de = NG.is_directed_edge(vi,vl);
		if ( (k_de && l_de) || (!k_de && !l_de) ) {
			if (debug_mode != DBG_SYMMETRIZE) {
				Ni.erase(Ni.begin()+j);
				directed_edge_info[vi].erase(directed_edge_info[vi].begin()+j);
			}
			return true;
		}
		else if (k_de) {
			if (debug_mode == DBG_SYMMETRIZE)
				draw_colored_edge(vi,vl,4,0,1,1);
			else
				Ni[j] = vl;
		} else {
			if (debug_mode == DBG_SYMMETRIZE)
				draw_colored_edge(vi,vk,4,0,1,1);
			else
				Ni[j] = vk;
		}
	}
	else if (debug_mode != DBG_SYMMETRIZE) {
		if (k+1 == nj) {
			Nj.push_back(vi);
			directed_edge_info[vj].push_back(0);
		}
		else {
			Nj.insert(Nj.begin()+k+1,vi);
			directed_edge_info[vj].insert(directed_edge_info[vj].begin()+k+1,0);
		}
	}
	return false;
}

void surface_reconstructor::symmetrize()
{
	if (directed_edge_info.empty()) {
		std::cout << "symmetrize of surface reconstructor only possible after construction of face info" << std::endl;
		return;
	}
	unsigned int vi, vj, j;
	// ensure that all is defined
	if (!ng || !pc)
		return;
	neighbor_graph& NG = *ng;
	unsigned int nr_removed = 0, nr_added = 0;
	for (vi=0; vi<NG.size(); ++vi) {
		// reference neighborhood Ni of vi
		const std::vector<Idx> &Ni = NG[vi];
		unsigned int n = (int) Ni.size();

		unsigned int k = n-1;
		for (j=0; j<n; ++j) {
			vj = Ni[j];
			if (NG.is_directed_edge(vj,vi))
				continue;
			if (symmetrize_directed_edge(vi,j)) {
				--n;
				--j;
				++nr_removed;
			}
			else
				++nr_added;
		}
	}
	std::cout << "removed " << nr_removed << " directed edges and symmetrized " << nr_added << " edges." << std::endl;
}


bool surface_reconstructor::delaunay_fan_filter(unsigned int vi, std::vector<Idx>& Mi, std::vector<unsigned char>& Ei) const
{
	unsigned int j;
	const std::vector<Idx>& Ni = ng->at(vi);
	const Pnt& pi = pc->pnt(vi);

	// construct local coordinate system
	Vec x,y,z = normalize(pc->nml(vi));
	if (!put_coordinate_system(vi,x,y))
		return false;

	// construct 2d neighborhood
	std::vector<P2d> N2d;
	N2d.resize(Ni.size());
	for (j=0; j<Ni.size(); ++j) {
		unsigned int vj = Ni[j];
		Vec zj = normalize(pc->nml(vj));
		Vec dij = pc->pnt(vj) - pi;
		float nx = dot(x,dij);
		float ny = dot(y,dij);
		float nz = dot(z,dij);
		float l_squared = nx*nx+ny*ny;
		float L_squared = (float)((1-z_weight)*l_squared+z_weight*nz*nz);
		float s  = sqrt(L_squared/l_squared);
		if (use_normal_weight) {
			float nn = compute_normal_quality(z,zj);
			if (nn < 0.00001f)
				nn = 0.00001f;
			s /= nn;
		}
		N2d[j] = P2d(nx*s,ny*s);
	}

	// constraint delaunay flipping in 1-ring
	Mi = Ni;
	Ei = directed_edge_info[vi];
	bool changed = true;
	while (changed) {
		changed = false;
		int k = (unsigned int) N2d.size()-1;
		for (j = 0; j < N2d.size(); ++j) {
			if ((Ei[j]&2) == 0 &&
				 !is_locally_delaunay(P2d(0.0f),N2d[k],N2d[j],N2d[(j+1)%Mi.size()])) {
				
				N2d.erase(N2d.begin()+j);
				Mi.erase(Mi.begin()+j);
				Ei.erase(Ei.begin()+j);
				changed = true;
				break;
			}
			k = j;
		}
	}
	return true;
}

void surface_reconstructor::delaunay_fan_neighbor_graph_filter()
{
	cgv::utils::progression prog("delaunay filter          ", (unsigned int)pc->get_nr_points(), 10);
	for (unsigned int vi=0; vi < pc->get_nr_points(); ++vi) {
		prog.step();
		std::vector<Idx> new_Ni;
		std::vector<unsigned char> new_Ei;
		if (!delaunay_fan_filter(vi, new_Ni, new_Ei))
			continue;
		// determine median edge length as reference length
		if (compute_reference_length_from_delaunay_filter) {
			unsigned int n = (unsigned int) new_Ni.size();
			static std::vector<Crd> sls;
			sls.resize(n);
			const Pnt& pi = pc->pnt(vi);
			for (unsigned int i=0; i<n; ++i)
				sls[i] = sqr_length(pi - pc->pnt(new_Ni[i]));
			std::sort(sls.begin(), sls.end());
			set_reference_length(vi,sqrt(sls.back()));
		}
		ng->at(vi) = new_Ni;
		directed_edge_info[vi] = new_Ei;
	}
}



/*
bool surface_reconstructor::follow_cycle(const graph_location& li, graph_location& lj)
{
	const Pnt& pi = pc->P[li.vi];
	lj.vi = ng->vi(li);
	const Pnt& pj = pc->P[lj.vi];
	Nml mi = cross(pj-pi,pc->N[l.vi]);

}
*/
void surface_reconstructor::cycle_filter()
{

	std::cout << "cycle_filter not implemented" << std::endl;
}

