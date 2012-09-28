#pragma once

#include <vector>
#include <deque>
#include <cgv/utils/progression.h>
#include <cgv/math/fvec.h>
#include <cgv/math/mfunc.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/media/mesh/streaming_mesh.h>

#include <cgv/media/lib_begin.h>

namespace cgv {
	namespace media {
		namespace mesh {

extern CGV_API int get_nr_cube_triangles(int idx);
extern CGV_API void put_cube_triangle(int idx, int t, int& vi, int& vj, int& vk);


/** data structure for the information that is cached per volume slice */
template <typename T>
struct slice_info
{
	unsigned int resx, resy;
	std::vector<T> values;
	std::vector<bool> flags;
	std::vector<int> indices;
	///
	slice_info(unsigned int _resx, unsigned int _resy) : resx(_resx), resy(_resy) {
		unsigned int n = resx*resy;
		values.resize(n);
		flags.resize(n);
		indices.resize(4*n);
	}
	///
	void init() {
		int n = resx*resy;
		for (int i=0; i<n; ++i)	indices[4*i+3] = -1;
	}
	///
	bool flag(int x, int y) const { return flags[y*resx+x]; }
	///
	int get_bit_code(int x, int y, int step=1) const {
		int i = y*resx+x;
		return (flags[i]?1:0) + (flags[i+step]?2:0) +
				 (flags[i+step*(resx+1)]?4:0) + (flags[i+step*resx]?8:0);
	}
	///
	const T& value(int x, int y) const { return values[y*resx+x]; }
	      T& value(int x, int y)       { return values[y*resx+x]; }
	/// set value and flag at once
	void set_value(int x, int y, T value, T iso_value) {
		int i = y*resx+x;
		values[i] = value;
		flags[i] = value>iso_value;
	}
	///
	const int& index(int x, int y, int e) const { return indices[4*(y*resx+x)+e]; }
			int& index(int x, int y, int e)		  { return indices[4*(y*resx+x)+e]; }
	///
	const int& snap_index(int x, int y) const { return indices[4*(y*resx+x)+3]; }
			int& snap_index(int x, int y)		 { return indices[4*(y*resx+x)+3]; }
};

/// class used to perform the marching cubes algorithm
template <typename X, typename T>
class marching_cubes : public streaming_mesh<X>
{
public:
	typedef streaming_mesh<X> base_type;
	/// points must have three components
	typedef cgv::math::fvec<X,3> pnt_type;
	/// vectors must have three components
	typedef cgv::math::fvec<X,3> vec_type;
private:
	pnt_type p;
	vec_type d;
	T iso_value;
protected:
	const cgv::math::v3_func<X,T>& func;
	X epsilon;
	X grid_epsilon;
public:
	/// construct marching cubes object
	marching_cubes(const cgv::math::v3_func<X,T>& _func,
				   streaming_mesh_callback_handler* _smcbh, 
				   const X& _grid_epsilon = 0.01f, 
				   const X& _epsilon = 1e-6f) : func(_func), epsilon(_epsilon), grid_epsilon(_grid_epsilon)
	{
		base_type::set_callback_handler(_smcbh);
	}
	/// construct a new vertex on an edge
	void construct_vertex(slice_info<T> *info_ptr_1, int i_1, int j_1, int e,
		                   slice_info<T> *info_ptr_2, int i_2, int j_2)
	{
		// read values at edge ends
		T v_1 = info_ptr_1->value(i_1,j_1);
		T v_2 = info_ptr_2->value(i_2,j_2);
		// from values compute affin location
		X f = (fabs(v_2-v_1) > epsilon) ? (X)(iso_value-v_1)/(v_2-v_1) : (X) 0.5;
		// check whether to snap to edge start
		int vi = base_type::get_nr_vertices();
		pnt_type q = p;
		if (f < grid_epsilon) {
			int vj = info_ptr_1->snap_index(i_1, j_1);
			if (vj != -1) {
				info_ptr_1->index(i_1, j_1, e) = vj;
				return;
			}
			info_ptr_1->snap_index(i_1, j_1) = vi;
			q(e) -= d(e);
		}
		else if (1-f < grid_epsilon) {
			int vj = info_ptr_2->snap_index(i_2, j_2);
			if (vj != -1) {
				info_ptr_1->index(i_1, j_1, e) = vj;
				return;
			}
			info_ptr_2->snap_index(i_2, j_2) = vi;
		}
		else
			q(e) -= (1-f)*d(e);

		info_ptr_1->index(i_1, j_1, e) = vi;
		this->new_vertex(q);
	}

	/// extract iso surface and send triangles to marching cubes handler
	void extract(const T& _iso_value,
				 const axis_aligned_box<X,3>& box,
				 unsigned int resx, unsigned int resy, unsigned int resz,
				 bool show_progress = false)
	{
		// prepare private members
		p = box.get_min_pnt();
		d = box.get_extent();
		d(0) /= (resx-1); d(1) /= (resy-1); d(2) /= (resz-1);
		iso_value = _iso_value;

		// prepare progression
		cgv::utils::progression prog;
		if (show_progress) prog.init("extraction", resz, 10);

		// construct two slice infos
		slice_info<T> slice_info_1(resx,resy), slice_info_2(resx,resy);
		slice_info<T> *slice_info_ptrs[2] = { &slice_info_1, &slice_info_2 };

		// iterate through all slices
		unsigned int nr_vertices[3] = { 0, 0, 0 };
		unsigned int i, j, k, n;
		for (k=0; k<resz; ++k, p(2) += d(2)) {
			n = (int)base_type::get_nr_vertices();
			// evaluate function on next slice and construct slice interior vertices
			slice_info<T> *info_ptr = slice_info_ptrs[k&1];
			info_ptr->init();
			for (j = 0, p(1) = box.get_min_pnt()(1); j < resy; ++j, p(1) += d(1))
				for (i = 0, p(0) = box.get_min_pnt()(0); i < resx; ++i, p(0)+=d(0)) {
					info_ptr->set_value(i,j,func.evaluate(p.to_vec()),iso_value);
					if (i > 0 && info_ptr->flag(i-1,j) != info_ptr->flag(i,j))
						construct_vertex(info_ptr, i-1,j, 0, info_ptr, i,j);
					if (j > 0 && info_ptr->flag(i,j-1) != info_ptr->flag(i,j))
						construct_vertex(info_ptr, i,j-1, 1, info_ptr, i,j);
				}
			// show progression
			if (show_progress) 
				prog.step();
			// if this is the first considered slice, construct the next one
			if (k != 0) {
				// get info of previous slice
				slice_info<T> *prev_info_ptr = slice_info_ptrs[1-(k&1)];
				// construct vertices on edges between previous and new slice
				for (j = 0, p(1) = box.get_min_pnt()(1); j < resy; ++j, p(1) += d(1))
					for (i = 0, p(0) = box.get_min_pnt()(0); i < resx; ++i, p(0)+=d(0))
						if (prev_info_ptr->flag(i,j) != info_ptr->flag(i,j))
							construct_vertex(prev_info_ptr, i,j, 2, info_ptr, i,j);

				// construct triangles
				for (j = 0; j < resy-1; ++j)
					for (i = 0; i < resx-1; ++i) {
						// compute the bit index for the current cube
						int idx = prev_info_ptr->get_bit_code(i,j)+
									 16*info_ptr->get_bit_code(i,j);
						// skip empty cubes
						if (idx == 0 || idx == 255)
							continue;
						// set edge vertices
						int vis[12] = {
							prev_info_ptr->index(i,j,0),
							prev_info_ptr->index(i+1,j,1),
							prev_info_ptr->index(i,j+1,0),
							prev_info_ptr->index(i,j,1),
							info_ptr->index(i,j,0),
							info_ptr->index(i+1,j,1),
							info_ptr->index(i,j+1,0),
							info_ptr->index(i,j,1),
							prev_info_ptr->index(i,j,2),
							prev_info_ptr->index(i+1,j,2),
							prev_info_ptr->index(i,j+1,2),
							prev_info_ptr->index(i+1,j+1,2) 
						};
						// lookup triangles and construct them
						int n = get_nr_cube_triangles(idx);
						for (int t=0; t<n; ++t) {
							int vi, vj, vk;
							put_cube_triangle(idx, t, vi, vj, vk);
							vi = vis[vi];
							vj = vis[vj];
							vk = vis[vk];
							if ( (vi != vj) && (vi != vk) && (vj != vk) )
								base_type::new_triangle(vk,vj,vi);
						}
					}
			}
			n = (int)base_type::get_nr_vertices()-n;
			nr_vertices[k%3] = n;
			n = nr_vertices[(k+2)%3];
			if (n > 0)
				base_type::drop_vertices(n);
		}
	}
};

		}
	}
}

#include <cgv/config/lib_end.h>
