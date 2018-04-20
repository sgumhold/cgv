#pragma once

#include <vector>
namespace cgv {
	namespace data {

template <typename Pnt, typename Idx = int>
class grid : public std::vector<std::vector<Idx> >
{
public:
	typename decltype(min_pnt[0]) Crd;
	typename decltype(max_pnt-min_pnt) Vec;
	std::size_t dim[3];
	Crd rmax;
	Crd rmax_sqr;
	Crd rmax_estimate_factor;
protected:
	Vec scale;
	Vec extent;
	Vec cell_extent;
	Pnt min_pnt, max_pnt;

	void init() {
		clear();
		resize(get_size());
	}
	void compute_scales() {
		extent = max_pnt-min_pnt;
		for (int c=0; c<3; ++c) {
			scale[c]       = dim[c]/extent[c];
			cell_extent[c] = extent[c]/dim[c];
		}
	}
public:
	grid(Idx dim = 20)                 { set_dim(dim); init(); }
	grid(Idx dimx, Idx dimy, Idx dimz) { set_dim(dimx,dimy,dimz); init(); }
	void clear() {
		std::vector<std::vector<Idx> >::clear();
		min_pnt = Pnt(-1,-1,-1);
		max_pnt = Pnt(1,1,1);
	}
	bool is_empty() const { return std::vector<std::vector<Idx> >::empty(); }
	void set_dim(Cnt dim) { set_dim(dim,dim,dim); }
	void set_dim(Cnt dimx, Cnt dimy, Cnt dimz) {
		dim[0] = dimx;
		dim[1] = dimy;
		dim[2] = dimz;
		init();
		compute_scales();
	}
	void set_box(const Pnt& _min_pnt, const Pnt& _max_pnt) {
		min_pnt = _min_pnt;
		max_pnt = _max_pnt;
		compute_scales();
	}
	template <typename Func>
	void compute_box(Idx nr_points, const Func& pnts) {
		min_pnt = max_pnt = pnts(0);
		for (Idx i=1; i<nr_points; ++i) {
			for (int c=0; c<3; ++c) {
				if (pnts(i)[c] < min_pnt[c])
					min_pnt[c] = pnts(i)[c];
				if (pnts(i)[c] > max_pnt[c])
					max_pnt[c] = pnts(i)[c];
			}
		}
		compute_scales();
	}
	/// before calling the build function, ensure that the bounding box has been set with set_box or computed with compute_box
	template <typename Func>
	void build(Idx nr_points, const Func& pnts) {
		init();
		rmax = (Crd) (rmax_estimate_factor*sqrt(2*(extent[0]*(extent[1]+extent[2])+extent[1]*extent[2])/nr_points));
		rmax_sqr = rmax*rmax;
		cout << "rmax = " << rmax << endl;
		set_dim((Idx)(extent[0]/rmax), (Idx)(extent[1]/rmax), (Idx)(extent[2]/rmax));
		for (Idx i=0; i<nr_points; ++i)
			insert(pnts(i), i);
	}
	Idx get_size() const { 
		return dim[0]*dim[1]*dim[2]; 
	}
	template <typename Jdx>
	Idx get_index(Jdx *ci) const { 
		return (ci[2]*dim[1]+ci[1])*dim[0]+ci[0]; 
	}
	Idx get_index(const Pnt& p) const { 
		Idx ci[3];
		for (int c = 0; c < 3; ++c) {
			ci[c] = (Idx) (scale[c]*(p[c]-min_pnt[c]));
			if (ci[c] >= (Idx)dim[c])
				ci[c] = dim[c]-1;
		}
		return get_index(ci);	
	}
	template <typename Jdx>
	void split_index(Idx idx, Jdx* ci) const {
		Idx n = dim[0]*dim[1];
		ci[2] = (Jdx)(idx / n);
		idx -= n*ci[2];
		ci[1] = (Jdx)(idx/dim[0]);
		ci[0] = (Jdx)(idx-dim[0]*ci[1]);
	}
	void put_grid_cell(Idx idx, const Pnt& _min_pnt, const Pnt& _max_pnt) const {
		Idx ci[3];
		split_index(idx, ci);
		for (int c=0; c<3; ++c)
			_min_pnt[c] = ci[c]/scale[c] + min_pnt[c];
		_max_pnt = _min_pnt+cell_extent;
	}
	void insert(const Pnt& p, Idx i) {
		Idx ci = get_index(p);
		if (ci >= size()) {
			std::cerr << "hit not available cell " << ci << endl;
			return;
		}
		at(ci).push_back(i);
	}
	void add_neighbors(Idx pi, const Pnt& p, Idx ci, const Func& pnts, vector<Idx>& N) const {
		for (Idx i=0; i<at(ci).size(); ++i) {
			Idx ni = at(ci)[i];
			if (ni != pi) {
				if (sqr_length(pnts(ni)-p) < rmax_sqr)
					N.push_back(ni);
			}
		}
	}
	template <typename Func>
	void extract_neighbors(Idx i, const Func& pnts, vector<Idx>& N) const
	{
		static const int d[3*26] = {
			1,0,0,
			0,1,0,
			0,0,1,
			-1,0,0,
			0,-1,0,
			0,0,-1,

			1,1,0,
			0,1,1,
			1,0,1,
			-1,1,0,
			0,-1,1,
			-1,0,1,
			1,-1,0,
			0,1,-1,
			1,0,-1,
			-1,-1,0,
			0,-1,-1,
			-1,0,-1,

			1,1,1,
			-1,1,1,
			1,-1,1,
			-1,-1,1,
			1,1,-1,
			-1,1,-1,
			1,-1,-1,
			-1,-1,-1
		};
		const Pnt& p = pnts(i);
		Idx ci = get_index(p);
		add_neighbors(i, p, ci, N);
		for (int j=0; j<26; ++j) {
			Idx cj = ci+d[3*j]+dim[0]*(d[3*j+1]+dim[1]*d[3*j+2]);
			if (cj >= 0 && cj < (Idx)size())
				add_neighbors(i, p, cj, N);
		}
	}
};

	}
}