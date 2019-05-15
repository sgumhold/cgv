
#define _USE_MATH_DEFINES
#include <cmath>
#include "surface_reconstructor.h"
#include <cgv/math/fvec.h>
#include <cgv/math/normal_estimation.h>


/// check if the edge opposite to the given corner is locally delaunay
bool surface_reconstructor::is_locally_delaunay(
	const P2d& p1, const P2d& p3, const P2d& p2, const P2d& p0) const
{
	const Crd& x0 = p0.x();
	const Crd& y0 = p0.y();
	      Crd  r0 = x0*x0+y0*y0;
	const Crd& x1 = p1.x();
	const Crd& y1 = p1.y();
	      Crd  r1 = x1*x1+y1*y1;
	const Crd& x2 = p2.x();
	const Crd& y2 = p2.y();
	      Crd  r2 = x2*x2+y2*y2;
	const Crd& x3 = p3.x();
	const Crd& y3 = p3.y();
	      Crd  r3 = x3*x3+y3*y3;
	Crd det = 
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

surface_reconstructor::Dir surface_reconstructor::compute_tangential_direction(unsigned int vi, unsigned int vj) const
{
	const Nml& n = pc->nml(vi);
	Dir d = pc->pnt(vj)-pc->pnt(vi);
	d = d-dot(d,n)*n;
	return normalize(d);
}

surface_reconstructor::Pnt surface_reconstructor::compute_corner_point(unsigned int vi, unsigned int vj, unsigned int vk, float weight) const
{
	const std::vector<Idx> &Ni = ng->at(vi);
	const Pnt& pi = pc->pnt(vi);
	unsigned int n = (int) Ni.size();
	const Pnt& pj = pc->pnt(vj);
	const Pnt& pk = pc->pnt(vk);
	return weight*pi + (0.5f*(1-weight))*(pj+pk);
}

surface_reconstructor::Pnt surface_reconstructor::compute_corner_point(unsigned int vi, unsigned int j, float weight) const
{
	const std::vector<Idx> &Ni = ng->at(vi);
	unsigned int n = (int) Ni.size();
	return compute_corner_point(vi,Ni[j],Ni[(j+1)%n],weight);
}

/// compute the normal of a triangle
surface_reconstructor::Nml surface_reconstructor::compute_triangle_normal(unsigned int vi, unsigned int vj, unsigned int vk) const
{
	const Pnt& pi = pc->pnt(vi);
	const Pnt& pj = pc->pnt(vj);
	const Pnt& pk = pc->pnt(vk);
	return normalize(cross(pj-pi,pk-pi));
}

bool surface_reconstructor::put_coordinate_system(unsigned int vi, unsigned int j, Vec& x, Vec& y) const
{
	if ((int)ng->size() <= vi || j >= ng->at(vi).size())
		return false;
	const Nml& n = pc->nml(vi);
	const Pnt& q = pc->pnt(ng->at(vi)[j]);
	y = normalize(cross(n,q-pc->pnt(vi)));
	x = normalize(cross(y,n));
	return true;
}

bool surface_reconstructor::put_coordinate_system(unsigned int vi, Vec& x, Vec& y) const
{
	return put_coordinate_system(vi, 0, x, y);
}

double surface_reconstructor::compute_tangential_corner_angle(unsigned int vi, const Dir& d0, unsigned int vj) const
{
	Dir   d  = compute_tangential_direction(vi, vj);
	float c = dot(d0,d);
	float s  = dot(cross(d0,d),pc->nml(vi));
	float a  = atan2(s,c);
	if (a < 0)
		a += (float)(2*M_PI);
	return a;
}

double surface_reconstructor::compute_tangential_corner_angle(unsigned int vi, unsigned int vj, unsigned int vk) const
{
	return compute_tangential_corner_angle(vi, compute_tangential_direction(vi,vj), vk);
}

double surface_reconstructor::compute_tangential_corner_angle(unsigned int vi, unsigned int j) const
{
	const std::vector<Idx>& Ni = ng->at(vi);
	return compute_tangential_corner_angle(vi, Ni[j], Ni[(j+1)%Ni.size()]);
}


double surface_reconstructor::compute_corner_angle(unsigned int vi, unsigned int vj, unsigned int vk) const
{
	Vec dij = normalize(pc->pnt(vj) - pc->pnt(vi));
	Vec dik = normalize(pc->pnt(vk) - pc->pnt(vi));
	Crd c = dot(dij,dik);
	Crd s = length(cross(dij,dik));
	Crd a  = atan2(s,c);
	if (a < 0)
		a += (Crd)(2*M_PI);
	if (a > M_PI)
		a = (Crd)(2*M_PI) - a;
	return a;
}

double surface_reconstructor::compute_corner_angle(unsigned int vi, unsigned int j) const
{
	const std::vector<Idx>& Ni = ng->at(vi);
	return compute_corner_angle(vi, Ni[j], Ni[(j+1)%Ni.size()]);
}


unsigned int surface_reconstructor::find_surrounding_corner(unsigned int vi, unsigned int vj) const
{
	const std::vector<Idx>& Ni = ng->at(vi);
	Dir d0 = compute_tangential_direction(vi, Ni[0]);
	unsigned int j = 0;
	double aj = compute_tangential_corner_angle(vi, d0, vj);
	do {
		++j;
		if (j < Ni.size()) {
			if (compute_tangential_corner_angle(vi, d0, Ni[j]) > aj)
				break;
		}
		else
			break;
	} while (true);
	return j-1;
}

/// copy the original normals to the secondary normals
void surface_reconstructor::toggle_normals()
{
	if (secondary_normals.empty()) {
		secondary_normals.resize(pc->get_nr_points());
		for (Idx i=0; i<(Idx)pc->get_nr_points(); ++i)
			secondary_normals[i] = pc->nml(i);
	}
	else {
		for (unsigned int vi=0; vi < pc->get_nr_points(); ++vi)
			std::swap(pc->nml(vi),secondary_normals[vi]);
	}
}

float get_rand()
{
	return (float)rand()/RAND_MAX;
}

float get_rand(float min, float max)
{
	return get_rand()*(max-min)+min;
}

void surface_reconstructor::jitter_points()
{
	if (!ng || ng->empty()) {
		std::cout << "cannot jitter points without neighbor graph" << std::endl;
		return;
	}
	for (unsigned int vi=0; vi < pc->get_nr_points(); ++vi) {
		Crd a = 0.01f*length(pc->pnt(vi)-pc->pnt(ng->at(vi)[0]));
		pc->pnt(vi) = pc->pnt(vi)+Vec(get_rand(-a,a),get_rand(-a,a),get_rand(-a,a));
	}
}

/// copy the original normals to the secondary normals
void surface_reconstructor::randomize_orientation()
{
	for (unsigned int vi=0; vi < pc->get_nr_points(); ++vi) {
		if (get_rand() > 0.5f)
			pc->nml(vi) = -pc->nml(vi);
	}
}

/// copy the original normals to the secondary normals
void surface_reconstructor::toggle_points()
{
	if (secondary_points.empty()) {
		secondary_points.resize(pc->get_nr_points());
		for (Idx i=0; i<(Idx)pc->get_nr_points(); ++i)
			secondary_points[i] = pc->pnt(i);
	}
	else {
		for (unsigned int vi=0; vi < pc->get_nr_points(); ++vi)
			std::swap(pc->pnt(vi),secondary_points[vi]);
	}
}

/// compute normals from neighbor graph and distance and normal weights
void surface_reconstructor::smooth_normals()
{
	unsigned int vi;
	unsigned int n = (unsigned int) pc->get_nr_points();
	std::vector<Nml> NS;
	NS.resize(pc->get_nr_points());
	Idx i;
	for (i=0; i<(Idx)pc->get_nr_points(); ++i)
		NS[i] = pc->nml(i);
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = pc->pnt(vi);
		const Nml& nml_i = pc->nml(vi);
		const std::vector<Idx> &Ni = ng->at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		Crd l0_sqr = cgv::math::sqr_length(pc->pnt(Ni[std::min(signed(ni)-1,6)]) - pi);
		Pnt center(0,0,0);
		Crd weight_sum = 0;
		Vec nml_avg(0,0,0);
		Vec ortho(0,0,0);
		Vec repulse(0,0,0);
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			Vec dij = pc->pnt(vj)-pc->pnt(vi);
			Crd lij_sqr = sqr_length(dij);
			Crd w_x = exp(-lij_sqr/l0_sqr);
			Crd w_n = compute_normal_quality(nml_i,pc->nml(vj));
			Crd w   = w_x*w_n;

			// compute area weighted normal
			dij = (1/sqrt(lij_sqr))*dij;
			Nml nml_ij = cross(dij,cross(nml_i,dij));

			// add contributions
			nml_avg += w*pc->nml(vj);
			ortho   += w*nml_ij;
			repulse += w*dij;
			center  += w*pc->pnt(vj);
			weight_sum += w;
		}
		center = (1.0f/weight_sum)*center;
		NS[vi] = normalize(pc->nml(vi) + 0.4f*normalize(
			     nml_avg
//		   +0.5f*ortho
//			-3*(dot(nml(vi), center - pnt(vi))/sqrt(l0_sqr))*repulse
			-repulse
			));
	}
	for (i=0; i<(Idx)pc->get_nr_points(); ++i)
		pc->nml(i) = NS[i];
}

/// recompute normals from neighbor graph and distance and normal weights
void surface_reconstructor::compute_weighted_normals(bool reorient)
{
	unsigned int vi;
	unsigned int n = (unsigned int) pc->get_nr_points();

	std::vector<Crd> weights;
	std::vector<Pnt> points;
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = pc->pnt(vi);
		const std::vector<Idx> &Ni = ng->at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		weights.resize(ni+1);
		points.resize(ni+1);
		weights[0] = 1;
		points[0] = pi;
		Crd l0_sqr = cgv::math::sqr_length(pc->pnt(Ni[std::min(signed(ni)-1,6)]) - pi);
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			Vec dij = pc->pnt(vj)-pc->pnt(vi);
			Crd lij_sqr = sqr_length(dij);
			Crd w_x = exp(-lij_sqr/l0_sqr);
			weights[j+1] = w_x;
			points[j+1] = pc->pnt(vj);
		}
		Nml new_nml;
		estimate_normal_wls((Cnt)points.size(), points[0], &weights[0], new_nml);
		if (reorient && (dot(new_nml,pc->nml(vi)) < 0))
			new_nml = -new_nml;
		pc->nml(vi) = new_nml;
	}
}

/// recompute normals from neighbor graph and distance and normal weights
void surface_reconstructor::compute_bilateral_weighted_normals(bool reorient)
{
	unsigned int vi;
	unsigned int n = (unsigned int) pc->get_nr_points();
	std::vector<Nml> NS;
	NS.resize(pc->get_nr_points());
	Idx i;
	for (i=0; i<(Idx)pc->get_nr_points(); ++i)
		NS[i] = pc->nml(i);

	std::vector<Crd> weights;
	std::vector<Pnt> points;
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = pc->pnt(vi);
		const Nml& nml_i = pc->nml(vi);
		const std::vector<Idx> &Ni = ng->at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		weights.resize(ni+1);
		points.resize(ni+1);
		weights[0] = 1;
		points[0] = pi;
		Crd l0_sqr = cgv::math::sqr_length(pc->pnt(Ni[std::min(signed(ni)-1,6)]) - pi);
		Crd err0_sqr = (Crd)(l0_sqr*noise_to_sampling_ratio*noise_to_sampling_ratio);
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			Vec dij = pc->pnt(vj)-pc->pnt(vi);
			Crd lij_sqr = sqr_length(dij);
			Crd w_x = exp(-lij_sqr/l0_sqr);
			Crd errij = dot(pc->nml(vj),dij)*dot(pc->nml(vj),dij);
			Crd w_n = exp(-errij/err0_sqr);
			Crd w   = w_x*w_n;
			weights[j+1] = w;
			points[j+1] = pc->pnt(vj);
		}
		estimate_normal_wls((Cnt)points.size(), points[0], &weights[0], NS[vi]);
		if (reorient && (dot(NS[vi],pc->nml(vi)) < 0))
			NS[vi] = -NS[vi];
	}
	for (i=0; i<(Idx)pc->get_nr_points(); ++i)
		pc->nml(i) = NS[i];
}


#include <cgv/data/union_find.h>
#include <limits>

struct neighbor_info 
{
	struct {
		unsigned  vj : 31;
		bool flip : 1;
	};
	neighbor_info(unsigned _vj = 0, bool _flip = false) : vj(_vj), flip(_flip) {}
};

struct edge_info 
{
	unsigned vi;
	struct {
		unsigned  vj : 31;
		bool flip : 1;
	};
	edge_info(unsigned _vi, unsigned _vj, bool _flip) : vi(_vi), vj(_vj), flip(_flip) {}
};

struct weighted_edge_info 
{
	unsigned vi;
	struct {
		unsigned  vj : 31;
		bool flip : 1;
	};
	float w;
	weighted_edge_info(unsigned _vi, unsigned _vj, float signed_w) : vi(_vi), vj(_vj), w(fabs(signed_w)), flip(signed_w < 0) {}
	bool operator < (const weighted_edge_info& ei) const 
	{
		return w < ei.w;
	}
};

/// recompute normals from neighbor graph and distance and normal weights
void surface_reconstructor::orient_normals()
{
	std::cout << "orienting normals\n=================" << std::endl;
	unsigned int vi;
	unsigned int n = (unsigned int) pc->get_nr_points();

	// compute weighted edges and initial point with smallest x-component
	std::cout << "computing weighted edges" << std::endl;
	std::vector<weighted_edge_info> E;
	Crd min_x = std::numeric_limits<Crd>::max();
	unsigned v0 = 0;
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = pc->pnt(vi);
		if (pi[0] < min_x) {
			min_x = pi[0];
			v0 = vi;
		}
		const Nml& nml_i = pc->nml(vi);
		const std::vector<Idx> &Ni = ng->at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			const Pnt& pj = pc->pnt(vj);
			const Nml& nml_j = pc->nml(vj);
			Vec d = normalize(pj-pi);
			Vec nml_ip = nml_i - 2*dot(nml_i,d)*d;
			E.push_back(weighted_edge_info(vi,vj,dot(nml_ip, nml_j)));
		}
	}

	std::cout << "construct MST" << std::endl;
	// compute MST as simple neighborgraph
	cgv::data::union_find uf(n);
	std::sort(E.begin(), E.end());
	std::vector<std::vector<neighbor_info> > MST(n);
	for (vi = 1; vi < n; ) {
		if (E.empty()) {
			std::cerr << "warning: impossible to build MST over neighbor graph " << n-vi <<  " unreachable vertices" << std::endl;
			break;
		}
		const weighted_edge_info& wei = E.back();
		if (uf.find(wei.vi) != uf.find(wei.vj)) {
			uf.unify(wei.vi,wei.vj);
			MST[wei.vi].push_back(neighbor_info(wei.vj, wei.flip));
			MST[wei.vj].push_back(neighbor_info(wei.vi, wei.flip));
			++vi;
		}
		E.pop_back();
	}
	E.clear();

	/*
	for (vi = 0; vi < n; ++vi) {
		(*ng)[vi].clear();
		for (unsigned i=0;i<MST[vi].size(); ++i)
			(*ng)[vi].push_back(MST[vi][i].vj);
	}*/

	std::cout << "flipping edges starting at v0=" << v0 << std::endl;
	unsigned nr = 0;
	// flip starting with v0
	std::vector<edge_info> Q;
	Q.push_back(edge_info(v0,-1,pc->nml(v0)[0]>0));
	if (pc->nml(v0)[0]>0)
		pc->nml(v0) = -pc->nml(v0);
	while (!Q.empty()) {
		edge_info ei = Q.back();
		Q.pop_back();
		const std::vector<neighbor_info>& ni = MST[ei.vi];
		for (unsigned i=0; i<ni.size(); ++i) {
			unsigned vj = ni[i].vj;
			if (vj == ei.vj)
				continue;
			if (ni[i].flip != ei.flip) {
				pc->nml(vj) = -pc->nml(vj);
				++nr;
			}
			Q.push_back(edge_info(vj,ei.vi,ei.flip ^ ni[i].flip));
		}
	}
	std::cout << "flipped " << nr << " edges" << std::endl;
}
