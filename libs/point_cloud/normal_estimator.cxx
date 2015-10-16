#include "normal_estimator.h"
#include "normal_estimation.h"
#include <cgv/math/functions.h>
#include <algorithm>

normal_estimator::normal_estimator(point_cloud& _pc, neighbor_graph& _ng) : pc(_pc), ng(_ng) 
{
	normal_quality_exp = 5.0f;
	smoothing_scale = 1.0f;
	noise_to_sampling_ratio = 0.1f;
	use_orientation = true;
}

/// compute geometric quality of a triangle
normal_estimator::Crd normal_estimator::compute_normal_quality(const Nml& n1, const Nml& n2) const
{
	Crd nml_comp;
	if (use_orientation)
		nml_comp = 0.5f*(dot(n1,n2)+1);
	else {
		nml_comp  = dot(n1,n2);
		nml_comp *= nml_comp;
	}
	return 0.5f*(cgv::math::erf(normal_quality_exp*(nml_comp-0.5f))+1.0f);
}


/// compute normals from neighbor graph and distance and normal weights
void normal_estimator::smooth_normals()
{
	if (!pc.has_normals())
		compute_weighted_normals(false);

	std::vector<Crd> weights;
	std::vector<Pnt> points;

	// copy current normals
	std::vector<Nml> NS;
	NS.resize(pc.get_nr_points());
	Idx i, n = (Idx) pc.get_nr_points();
	for (i = 0; i < n; ++i)
		NS[i] = pc.nml(i);

	for (Idx vi = 0; vi < (Idx)pc.get_nr_points(); ++vi) {
		const Pnt& pi = pc.pnt(vi);
		const Nml& nml_i = pc.nml(vi);
		const std::vector<Idx> &Ni = ng.at(vi);
		unsigned ni = (unsigned) Ni.size();
		Crd l0_sqr = sqr_length(pc.pnt(Ni[std::min((int)ni-1,6)]) - pi)*smoothing_scale*smoothing_scale;
		Pnt center(0,0,0);
		Crd weight_sum = 0;
		Dir nml_avg(0,0,0);
		Dir ortho(0,0,0);
		Dir repulse(0,0,0);
		for (unsigned j=0; j < ni; ++j) {
			Idx vj = Ni[j];
			Dir dij = pc.pnt(vj)-pc.pnt(vi);
			Crd lij_sqr = sqr_length(dij);
			Crd w_x = exp(-lij_sqr/l0_sqr);
			Crd w_n = compute_normal_quality(nml_i,pc.nml(vj));
			Crd w   = w_x*w_n;

			// compute area weighted normal
			dij = (1/sqrt(lij_sqr))*dij;
			Nml nml_ij = cross(dij,cross(nml_i,dij));

			// add contributions
			nml_avg += w*pc.nml(vj);
			ortho   += w*nml_ij;
			repulse += w*dij;
			center  += w*pc.pnt(vj);
			weight_sum += w;
		}
		center = (1.0f/weight_sum)*center;
		NS[vi] = normalize(pc.nml(vi) + 0.4f*normalize(
			     nml_avg
//		   +0.5f*ortho
//			-3*(dot(N[vi], center - P[vi])/sqrt(l0_sqr))*repulse
			-repulse
			));
	}
	for (i = 0; i < n; ++i)
		pc.nml(i) = NS[i];
}

/// recompute normals from neighbor graph and distance and normal weights
void normal_estimator::compute_weighted_normals(bool reorient)
{
	if (!pc.has_normals()) {
		pc.create_normals();
		reorient = false;
	}
	std::vector<Crd> weights;
	std::vector<Pnt> points;
	for (Idx vi = 0; vi < (Idx)pc.get_nr_points(); ++vi) {
		const Pnt& pi = pc.pnt(vi);
		const std::vector<Idx> &Ni = ng.at(vi);
		unsigned ni = (unsigned) Ni.size();
		weights.resize(ni+1);
		points.resize(ni+1);
		weights[0] = 1;
		points[0] = pi;
		Crd l0_sqr = sqr_length(pc.pnt(Ni[std::min((int)ni-1,6)]) - pi)*smoothing_scale*smoothing_scale;
		for (unsigned j=0; j < ni; ++j) {
			Idx vj = Ni[j];
			Dir dij = pc.pnt(vj)-pc.pnt(vi);
			Crd lij_sqr = sqr_length(dij);
			Crd w_x = exp(-lij_sqr/l0_sqr);
			weights[j+1] = w_x;
			points[j+1] = pc.pnt(vj);
		}
		Nml new_nml;
		estimate_normal_wls((unsigned)points.size(), points[0], &weights[0], new_nml);
		if (reorient && (dot(new_nml,pc.nml(vi)) < 0))
			new_nml = -new_nml;
		pc.nml(vi) = new_nml;
	}
}

/// recompute normals from neighbor graph and distance and normal weights
void normal_estimator::compute_bilateral_weighted_normals(bool reorient)
{
	if (!pc.has_normals())
		compute_weighted_normals(reorient);

	std::vector<Crd> weights;
	std::vector<Pnt> points;

	// copy current normals
	std::vector<Nml> NS;
	NS.resize(pc.get_nr_points());
	Idx i, n = (Idx) pc.get_nr_points();
	for (i = 0; i < n; ++i)
		NS[i] = pc.nml(i);

	for (Idx vi = 0; vi < (Idx)pc.get_nr_points(); ++vi) {
		const Pnt& pi = pc.pnt(vi);
		const Nml& nml_i = pc.nml(vi);
		const std::vector<Idx> &Ni = ng.at(vi);
		unsigned ni = (unsigned) Ni.size();
		weights.resize(ni+1);
		points.resize(ni+1);
		weights[0] = 1;
		points[0] = pi;
		Crd l0_sqr = sqr_length(pc.pnt(Ni[std::min((int)ni-1,6)]) - pi)*smoothing_scale*smoothing_scale;
		for (unsigned j=0; j < ni; ++j) {
			Idx vj = Ni[j];
			Dir dij = pc.pnt(vj)-pc.pnt(vi);
			Crd lij_sqr = sqr_length(dij);
			Crd w_x = exp(-lij_sqr/l0_sqr);
			Crd w_n = compute_normal_quality(nml_i,pc.nml(vj));
			Crd w   = w_x*w_n;
			weights[j+1] = w;
			points[j+1] = pc.pnt(vj);
		}
		estimate_normal_wls((unsigned)points.size(), points[0], &weights[0], NS[vi]);
		if (reorient && (dot(NS[vi],pc.nml(vi)) < 0))
			NS[vi] = -NS[vi];
	}
	for (i = 0; i < n; ++i)
		pc.nml(i) = NS[i];
}

/// recompute normals from neighbor graph and distance and normal weights
void normal_estimator::compute_plane_bilateral_weighted_normals(bool reorient)
{
	if (!pc.has_normals())
		compute_weighted_normals(reorient);

	std::vector<Crd> weights;
	std::vector<Pnt> points;

	// copy current normals
	std::vector<Nml> NS;
	NS.resize(pc.get_nr_points());
	Idx i, n = (Idx) pc.get_nr_points();
	for (i = 0; i < n; ++i)
		NS[i] = pc.nml(i);

	for (Idx vi = 0; vi < (Idx)pc.get_nr_points(); ++vi) {
		const Pnt& pi = pc.pnt(vi);
		const Nml& nml_i = pc.nml(vi);
		const std::vector<Idx> &Ni = ng.at(vi);
		unsigned ni = (unsigned) Ni.size();
		weights.resize(ni+1);
		points.resize(ni+1);
		weights[0] = 1;
		points[0] = pi;
		Crd l0_sqr = sqr_length(pc.pnt(Ni[std::min((int)ni-1,6)]) - pi)*smoothing_scale*smoothing_scale;
		Crd err0_sqr = l0_sqr*noise_to_sampling_ratio*noise_to_sampling_ratio;
		for (unsigned j=0; j < ni; ++j) {
			Idx vj = Ni[j];
			Dir dij = pc.pnt(vj)-pc.pnt(vi);
			Crd lij_sqr = sqr_length(dij);
			Crd w_x = exp(-lij_sqr/l0_sqr);
			Crd errij = dot(pc.nml(vj),dij)*dot(pc.nml(vj),dij);
			Crd w_n = exp(-errij/err0_sqr);
			Crd w   = w_x*w_n;
			weights[j+1] = w;
			points[j+1] = pc.pnt(vj);
		}
		estimate_normal_wls((unsigned)points.size(), points[0], &weights[0], NS[vi]);
		if (reorient && (dot(NS[vi],pc.nml(vi)) < 0))
			NS[vi] = -NS[vi];
	}
	for (i = 0; i < n; ++i)
		pc.nml(i) = NS[i];
}

#include <cgv/math/union_find.h>
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

/// orient normals towards given point
void normal_estimator::orient_normals(const Pnt& view_point)
{
	if (!pc.has_normals())
		compute_weighted_normals(false);

	for (Idx vi = 0; vi < (Idx)pc.get_nr_points(); ++vi) {
		if (dot(pc.nml(vi),view_point-pc.pnt(vi)) < 0)
			pc.nml(vi) = -pc.nml(vi);
	}
}


/// recompute normals from neighbor graph and distance and normal weights
void normal_estimator::orient_normals()
{
	if (!pc.has_normals())
		compute_weighted_normals(false);
	std::cout << "orienting normals\n=================" << std::endl;

	// compute weighted edges and initial point with smallest x-component
	std::cout << "computing weighted edges" << std::endl;
	std::vector<weighted_edge_info> E;
	Crd min_x = std::numeric_limits<Crd>::max();
	Idx vi, v0 = 0;
	for (vi = 0; vi < (Idx)pc.get_nr_points(); ++vi) {
		const Pnt& pi = pc.pnt(vi);
		if (pi[0] < min_x) {
			min_x = pi(0);
			v0 = vi;
		}
		const Nml& nml_i = pc.nml(vi);
		const std::vector<Idx> &Ni = ng.at(vi);
		unsigned ni = (unsigned) Ni.size();
		for (unsigned j=0; j < ni; ++j) {
			Idx vj = Ni[j];
			const Pnt& pj = pc.pnt(vj);
			const Nml& nml_j = pc.nml(vj);
			Dir d = normalize(pj-pi);
			Dir nml_ip = nml_i - 2*dot(nml_i,d)*d;
			E.push_back(weighted_edge_info(vi,vj,dot(nml_ip, nml_j)));
		}
	}

	std::cout << "construct MST" << std::endl;
	// compute MST as simple neighborgraph
	cgv::math::union_find uf(pc.get_nr_points());
	std::sort(E.begin(), E.end());
	std::vector<std::vector<neighbor_info> > MST(pc.get_nr_points());
	for (vi = 1; vi < (Idx)pc.get_nr_points(); ) {
		if (E.empty()) {
			std::cerr << "warning: impossible to build MST over neighbor graph " << pc.get_nr_points()-vi <<  " unreachable vertices" << std::endl;
			break;
		}
		const weighted_edge_info& wei = E.back();
		if (uf.find(wei.vi) != uf.find(wei.vj)) {
			uf.unite(wei.vi,wei.vj);
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
	Q.push_back(edge_info(v0,-1,pc.nml(v0)(0)>0));
	if (pc.nml(v0)(0)>0)
		pc.nml(v0) = -pc.nml(v0);
	while (!Q.empty()) {
		edge_info ei = Q.back();
		Q.pop_back();
		const std::vector<neighbor_info>& ni = MST[ei.vi];
		for (unsigned i=0; i<ni.size(); ++i) {
			unsigned vj = ni[i].vj;
			if (vj == ei.vj)
				continue;
			if (ni[i].flip != ei.flip) {
				pc.nml(vj) = -pc.nml(vj);
				++nr;
			}
			Q.push_back(edge_info(vj,ei.vi,ei.flip ^ ni[i].flip));
		}
	}
	std::cout << "flipped " << nr << " edges" << std::endl;
}
