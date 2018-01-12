#pragma once

#include <vector>
#include <deque>
#include <cgv/utils/progression.h>
#include <cgv/math/qem.h>
#include <cgv/math/mfunc.h>
#include <cgv/media/axis_aligned_box.h>
#include "streaming_mesh.h"

namespace cgv {
	namespace media {
		namespace mesh {

			template <typename T>
			struct greater_equal
			{
				T reference_value;
				greater_equal(const T& _value) : reference_value(_value) {}
				bool operator () (const T& _value) const { return _value >= reference_value; }
			};

			template <typename T>
			struct equal
			{
				T reference_value;
				equal(const T& _value) : reference_value(_value) {}
				bool operator () (const T& _value) const { return _value == reference_value; }
			};


/** data structure for the information that is cached per volume slice bz the cuberille algorithm */
template <typename T, class P>
struct c_slice_info
{
	unsigned resx, resy;
	unsigned nr_vertices;
	std::vector<bool> flags;
	std::vector<int> indices;
	///
	c_slice_info(unsigned int _resx, unsigned int _resy) : resx(_resx), resy(_resy) {
		unsigned int n = resx*resy;
		flags.resize(n, false);
		indices.resize(n, -1);
		nr_vertices = 0;
	}
	///
	void init() {
		std::fill(flags.begin(), flags.end(), false);
		std::fill(indices.begin(), indices.end(), -1);
		nr_vertices = 0;
	}
	int linear_index(int x, int y) const { return y*resx + x; }
	///
	bool flag(int x, int y) const { return (x>=0) && (y>=0) && flags[linear_index(x, y)]; }
	///
	void set_flag(int x, int y, bool flag) { flags[linear_index(x, y)] = flag; }
	///
	const int& index(int x, int y) const { return indices[linear_index(x, y)]; }
	void set_index(int x, int y, int idx)		 { 
		indices[linear_index(x,y)] = idx;
		++nr_vertices;
	}
};

/// class used to perform the marching cubes algorithm
template <typename X, typename T, class P>
class cuberille : public streaming_mesh<X>
{
public:
	typedef streaming_mesh<X> base_type;
	/// points must have three components
	typedef cgv::math::fvec<X,3> pnt_type;
	/// vectors must have three components
	typedef cgv::math::fvec<X,3> vec_type;
private:
	pnt_type p, minp;
	unsigned int resx, resy, resz;
	vec_type d;
	const P& pred;
protected:
	const cgv::math::v3_func<X,T>& func;
public:
	/// construct dual contouring object
	cuberille(const cgv::math::v3_func<X,T>& _func,
			  streaming_mesh_callback_handler* _smcbh, 
		      const P& _pred) :
	func(_func), pred(_pred)
	{
		base_type::set_callback_handler(_smcbh);
	}
	/// construct a quadrilateral
	void generate_edge_quad(int vi, int vj, int vk, int vl, bool reorient)
	{
		if (reorient)
			base_type::new_quad(vi, vl, vk, vj);
		else
			base_type::new_quad(vi, vj, vk, vl);
	}
	/// generate all vertices needed in the given slice I[0] with I[1] being the previous slice
	void process_slice(c_slice_info<T, P>* I[2])
	{
		// init slice info
		I[0]->init();
		// iterate voxels of slice to create slice vertices
		unsigned i, j;
		for (j = 0, p(1) = minp(1); j <= resy; ++j, p(1) += d(1)) {
			for (i = 0, p(0) = minp(0); i <= resx; ++i, p(0) += d(0)) {
				// set voxel flag
				I[0]->set_flag(i, j, i < resx && j < resy && pred(func.evaluate(p.to_vec())));
				// and check whether assigned vertex is needed
				bool need_vertex = false;
				need_vertex = need_vertex || (I[0]->flag(i, j) != I[1]->flag(i, j));     // z(x0,y0)
				need_vertex = need_vertex || (I[0]->flag(i, j) != I[0]->flag(i, j - 1)); // y(x0,z0)
				need_vertex = need_vertex || (I[1]->flag(i, j) != I[1]->flag(i, j - 1)); // y(x0,z1)
				need_vertex = need_vertex || (I[0]->flag(i, j) != I[0]->flag(i - 1, j)); // x(y0,z0)
				need_vertex = need_vertex || (I[1]->flag(i, j) != I[1]->flag(i - 1, j)); // x(y0,z1)
				if (i > 0) {
					need_vertex = need_vertex || (I[0]->flag(i - 1, j) != I[1]->flag(i - 1, j));     // z(x1,y0)
					need_vertex = need_vertex || (I[0]->flag(i - 1, j) != I[0]->flag(i - 1, j - 1)); // y(x1,z0)
					need_vertex = need_vertex || (I[1]->flag(i - 1, j) != I[1]->flag(i - 1, j - 1)); // y(x1,z1)
					if (j > 0) {
						need_vertex = need_vertex || (I[0]->flag(i - 1, j - 1) != I[1]->flag(i - 1, j - 1)); // z(x1,y1)
					}
				}
				if (j > 0) {
					need_vertex = need_vertex || (I[0]->flag(i, j - 1) != I[1]->flag(i - 1, j));     // z(x0,y1)
					need_vertex = need_vertex || (I[0]->flag(i, j - 1) != I[0]->flag(i - 1, j - 1)); // x(y1,z0)
					need_vertex = need_vertex || (I[1]->flag(i, j - 1) != I[1]->flag(i - 1, j - 1)); // x(y1,z1)
				}
				// create vertex if necessary
				if (need_vertex)
					I[0]->set_index(i, j, new_vertex(p - T(0.5)*d));
			}
		}
		// iterate voxels again to create edge quads
		for (j = 0, p(1) = minp(1); j <= resy; ++j, p(1) += d(1)) {
			for (i = 0, p(0) = minp(0); i <= resx; ++i, p(0) += d(0)) {
				if ((j < resy) && (I[1]->flag(i - 1, j) != I[1]->flag(i, j)))
					generate_edge_quad(I[1]->index(i, j), I[1]->index(i, j + 1), I[0]->index(i, j + 1), I[0]->index(i, j), I[1]->flag(i, j));
				if ((i < resx) && (I[1]->flag(i, j - 1) != I[1]->flag(i, j)))
					generate_edge_quad(I[1]->index(i, j), I[0]->index(i, j), I[0]->index(i + 1, j), I[1]->index(i + 1, j), I[1]->flag(i, j));
				if ((i < resx) && (j < resy) && (I[1]->flag(i, j) != I[0]->flag(i, j)))
					generate_edge_quad(I[0]->index(i, j), I[0]->index(i + 1, j), I[0]->index(i + 1, j + 1), I[0]->index(i, j + 1), I[0]->flag(i, j));
			}
		}
	}
	/// extract iso surface and send quads to streaming mesh handler
	void extract(const axis_aligned_box<X,3>& box,
				 unsigned int _resx, unsigned int _resy, unsigned int _resz,
				 bool show_progress = false)
	{
		// prepare private members
		resx = _resx; resy = _resy; resz = _resz;
		minp = p = box.get_min_pnt();
		d = box.get_extent();
		d(0) /= (resx-1); d(1) /= (resy-1); d(2) /= (resz-1);

		// prepare progression
		cgv::utils::progression prog;
		if (show_progress) 
			prog.init("extraction", resz+1, 10);

		// construct three slice infos
		c_slice_info<T, P> slice_info_1(resx+1,resy+1), slice_info_2(resx+1,resy+1);
		c_slice_info<T, P> *I[2] = { &slice_info_1, &slice_info_2 };
		for (unsigned k=0; k<=resz; ++k, p(2) += d(2)) {
			process_slice(I);
			base_type::drop_vertices(I[1]->nr_vertices);
			// show progression
			if (show_progress)
				prog.step();
			std::swap(I[0], I[1]);
		}
	}
};
		}
	}
}
