#include "normal_estimator.h"
#include <cgv/math/functions.h>

#include <cgv/math/mat.h>
#include <cgv/math/eig.h>
#include <cgv/math/point_operations.h>

void estimate_normal_ls(unsigned nr_points, const float* _points, float* _normal, float* _evs = 0)
{
	cgv::math::mat<float> points;
	points.set_extern_data(3, nr_points, const_cast<float*>(_points));

	cgv::math::vec<float> normal;
	normal.set_extern_data(3, _normal);

	cgv::math::mat<float> covmat;
	cgv::math::vec<float> mean;

	cgv::math::covmat_and_mean(points,covmat,mean);
	cgv::math::mat<double> dcovmat(covmat), v;
	cgv::math::diag_mat<double> d;
	cgv::math::eig_sym(dcovmat,v,d);

	normal = cgv::math::vec<float>(normalize(v.col(2)));
	if (_evs) {
		_evs[0] = (float)d(0);		
		_evs[1] = (float)d(1);		
		_evs[2] = (float)d(2);
	}
}

void estimate_normal_wls(unsigned nr_points, const float* _points, const float* _weights, float* _normal, float* _evs = 0)
{
	cgv::math::mat<float> points;
	points.set_extern_data(3, nr_points, const_cast<float*>(_points));
	cgv::math::vec<float> weights;
	weights.set_extern_data(nr_points, const_cast<float*>(_weights));
	cgv::math::vec<float> normal;
	normal.set_extern_data(3, _normal);

	cgv::math::mat<float> covmat;
	cgv::math::vec<float> mean;

	

	cgv::math::weighted_covmat_and_mean(weights,points,covmat,mean);
	cgv::math::mat<double> dcovmat(covmat), v;
	cgv::math::diag_mat<double> d;
	cgv::math::eig_sym(dcovmat,v,d);

	normal = cgv::math::vec<float>(normalize(v.col(2)));

	if (_evs) {
		_evs[0] = (float)d(0);		
		_evs[1] = (float)d(1);		
		_evs[2] = (float)d(2);
	}
}

normal_estimator::normal_estimator(point_cloud& _pc, neighbor_graph& _ng) : pc(_pc), ng(_ng) 
{
	normal_quality_exp = 5.0f;
	smoothing_scale = 1.0f;
	noise_to_sampling_ratio = 0.1f;
	use_orientation = true;
}

/// compute geometric quality of a triangle
normal_estimator::coord_type normal_estimator::compute_normal_quality(const Nml& n1, const Nml& n2) const
{
	coord_type nml_comp;
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
	unsigned int vi;
	const std::vector<Pnt>& P = pc.P;
	std::vector<Nml>& N = pc.N;
	unsigned int n = (unsigned int) N.size();
	std::vector<Nml> NS = N;
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = P[vi];
		const Nml& nml_i = N[vi];
		const std::vector<unsigned int> &Ni = ng.at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		coord_type l0_sqr = sqr_length(P[Ni[__min(ni-1,6)]] - pi)*smoothing_scale*smoothing_scale;
		Pnt center(0,0,0);
		coord_type weight_sum = 0;
		Vec nml_avg(0,0,0);
		Vec ortho(0,0,0);
		Vec repulse(0,0,0);
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			Vec dij = P[vj]-P[vi];
			coord_type lij_sqr = sqr_length(dij);
			coord_type w_x = exp(-lij_sqr/l0_sqr);
			coord_type w_n = compute_normal_quality(nml_i,N[vj]);
			coord_type w   = w_x*w_n;

			// compute area weighted normal
			dij = (1/sqrt(lij_sqr))*dij;
			Nml nml_ij = cross(dij,cross(nml_i,dij));

			// add contributions
			nml_avg += w*N[vj];
			ortho   += w*nml_ij;
			repulse += w*dij;
			center  += w*P[vj];
			weight_sum += w;
		}
		center = (1.0f/weight_sum)*center;
		NS[vi] = normalize(N[vi] + 0.4f*normalize(
			     nml_avg
//		   +0.5f*ortho
//			-3*(dot(N[vi], center - P[vi])/sqrt(l0_sqr))*repulse
			-repulse
			));
	}
	N = NS;
}

/// recompute normals from neighbor graph and distance and normal weights
void normal_estimator::compute_weighted_normals(bool reorient)
{
	unsigned int vi;
	const std::vector<Pnt>& P = pc.P;
	unsigned int n = (unsigned int) P.size();
	std::vector<Nml>& N = pc.N;
	if (N.size() != n)
		N.resize(n);

	std::vector<coord_type> weights;
	std::vector<Pnt> points;
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = P[vi];
		const std::vector<unsigned int> &Ni = ng.at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		weights.resize(ni+1);
		points.resize(ni+1);
		weights[0] = 1;
		points[0] = pi;
		coord_type l0_sqr = sqr_length(P[Ni[__min(ni-1,6)]] - pi)*smoothing_scale*smoothing_scale;
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			Vec dij = P[vj]-P[vi];
			coord_type lij_sqr = sqr_length(dij);
			coord_type w_x = exp(-lij_sqr/l0_sqr);
			weights[j+1] = w_x;
			points[j+1] = P[vj];
		}
		Nml new_nml;
		estimate_normal_wls(points.size(), points[0], &weights[0], new_nml);
		if (reorient && (dot(new_nml,N[vi]) < 0))
			new_nml = -new_nml;
		N[vi] = new_nml;
	}
}

/// recompute normals from neighbor graph and distance and normal weights
void normal_estimator::compute_bilateral_weighted_normals(bool reorient)
{
	unsigned int vi;
	const std::vector<Pnt>& P = pc.P;
	std::vector<Nml>& N = pc.N;
	unsigned int n = (unsigned int) N.size();
	std::vector<Nml> NS = N;

	std::vector<coord_type> weights;
	std::vector<Pnt> points;
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = P[vi];
		const Nml& nml_i = N[vi];
		const std::vector<unsigned int> &Ni = ng.at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		weights.resize(ni+1);
		points.resize(ni+1);
		weights[0] = 1;
		points[0] = pi;
		coord_type l0_sqr = sqr_length(P[Ni[__min(ni-1,6)]] - pi)*smoothing_scale*smoothing_scale;
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			Vec dij = P[vj]-P[vi];
			coord_type lij_sqr = sqr_length(dij);
			coord_type w_x = exp(-lij_sqr/l0_sqr);
			coord_type w_n = compute_normal_quality(nml_i,N[vj]);
			coord_type w   = w_x*w_n;
			weights[j+1] = w;
			points[j+1] = P[vj];
		}
		estimate_normal_wls(points.size(), points[0], &weights[0], NS[vi]);
		if (reorient && (dot(NS[vi],N[vi]) < 0))
			NS[vi] = -NS[vi];
	}
	N = NS;
}

/// recompute normals from neighbor graph and distance and normal weights
void normal_estimator::compute_plane_bilateral_weighted_normals(bool reorient)
{
	unsigned int vi;
	const std::vector<Pnt>& P = pc.P;
	std::vector<Nml>& N = pc.N;
	unsigned int n = (unsigned int) N.size();
	std::vector<Nml> NS = N;

	std::vector<coord_type> weights;
	std::vector<Pnt> points;
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = P[vi];
		const Nml& nml_i = N[vi];
		const std::vector<unsigned int> &Ni = ng.at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		weights.resize(ni+1);
		points.resize(ni+1);
		weights[0] = 1;
		points[0] = pi;
		coord_type l0_sqr = sqr_length(P[Ni[__min(ni-1,6)]] - pi)*smoothing_scale*smoothing_scale;
		coord_type err0_sqr = l0_sqr*noise_to_sampling_ratio*noise_to_sampling_ratio;
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			Vec dij = P[vj]-P[vi];
			coord_type lij_sqr = sqr_length(dij);
			coord_type w_x = exp(-lij_sqr/l0_sqr);
			coord_type errij = dot(N[vj],dij)*dot(N[vj],dij);
			coord_type w_n = exp(-errij/err0_sqr);
			coord_type w   = w_x*w_n;
			weights[j+1] = w;
			points[j+1] = P[vj];
		}
		estimate_normal_wls(points.size(), points[0], &weights[0], NS[vi]);
		if (reorient && (dot(NS[vi],N[vi]) < 0))
			NS[vi] = -NS[vi];
	}
	N = NS;
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

	unsigned int vi;
	const std::vector<Pnt>& P = pc.P;
	std::vector<Nml>& N = pc.N;
	unsigned int n = (unsigned int) N.size();
	for (vi = 0; vi < n; ++vi) {
		if (dot(N[vi],view_point-P[vi]) < 0)
			N[vi] = -N[vi];
	}
}


/// recompute normals from neighbor graph and distance and normal weights
void normal_estimator::orient_normals()
{
	if (!pc.has_normals())
		compute_weighted_normals(false);
	std::cout << "orienting normals\n=================" << std::endl;
	unsigned int vi;
	const std::vector<Pnt>& P = pc.P;
	std::vector<Nml>& N = pc.N;
	unsigned int n = (unsigned int) N.size();

	// compute weighted edges and initial point with smallest x-component
	std::cout << "computing weighted edges" << std::endl;
	std::vector<weighted_edge_info> E;
	coord_type min_x = std::numeric_limits<coord_type>::max();
	unsigned v0 = 0;
	for (vi = 0; vi < n; ++vi) {
		const Pnt& pi = P[vi];
		if (pi[0] < min_x) {
			min_x = pi[0];
			v0 = vi;
		}
		const Nml& nml_i = N[vi];
		const std::vector<unsigned int> &Ni = ng.at(vi);
		unsigned int ni = (unsigned int) Ni.size();
		for (unsigned int j=0; j < ni; ++j) {
			unsigned int vj = Ni[j];
			const Pnt& pj = P[vj];
			const Nml& nml_j = N[vj];
			Vec d = normalize(pj-pi);
			Vec nml_ip = nml_i - 2*dot(nml_i,d)*d;
			E.push_back(weighted_edge_info(vi,vj,dot(nml_ip, nml_j)));
		}
	}

	std::cout << "construct MST" << std::endl;
	// compute MST as simple neighborgraph
	cgv::math::union_find uf(n);
	std::sort(E.begin(), E.end());
	std::vector<std::vector<neighbor_info> > MST(n);
	for (vi = 1; vi < n; ) {
		if (E.empty()) {
			std::cerr << "warning: impossible to build MST over neighbor graph " << n-vi <<  " unreachable vertices" << std::endl;
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
	Q.push_back(edge_info(v0,-1,N[v0][0]>0));
	if (N[v0][0]>0)
		N[v0] = -N[v0];
	while (!Q.empty()) {
		edge_info ei = Q.back();
		Q.pop_back();
		const std::vector<neighbor_info>& ni = MST[ei.vi];
		for (unsigned i=0; i<ni.size(); ++i) {
			unsigned vj = ni[i].vj;
			if (vj == ei.vj)
				continue;
			if (ni[i].flip != ei.flip) {
				N[vj] = -N[vj];
				++nr;
			}
			Q.push_back(edge_info(vj,ei.vi,ei.flip ^ ni[i].flip));
		}
	}
	std::cout << "flipped " << nr << " edges" << std::endl;
}
