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

/// information stored per cell
template <typename T>
struct cell_info {
	cgv::math::qem<T> Q;
	cgv::math::fvec<T,3> center;
	int count;
};


/** data structure for the information that is cached per volume slice */
template <typename T>
struct dc_slice_info
{
	unsigned int resx, resy;
	std::vector<T> values;
	std::vector<bool> flags;
	std::vector<int> indices;
	typedef cgv::math::qem<T> qem_type;
	typedef cgv::math::fvec<T,3> pnt_type;
	std::vector<cell_info<T> > cell_infos;
	///
	dc_slice_info(unsigned int _resx, unsigned int _resy) : resx(_resx), resy(_resy) {
		unsigned int n = resx*resy;
		values.resize(n);
		flags.resize(n);
		indices.resize(n);
		cell_infos.resize(n);
	}
	///
	void init() {
		int n = resx*resy;
		for (int i=0; i<n; ++i)	{
			indices[i] = -1;
			cell_infos[i].Q = qem_type();
			cell_infos[i].count = 0;
			cell_infos[i].center = pnt_type();
		}
	}
	///
	bool flag(int x, int y) const             { return flags[y*resx+x]; }
	///
	const T& value(int x, int y) const        { return values[y*resx+x]; }
	      T& value(int x, int y)              { return values[y*resx+x]; }
	/// set value and flag at once
	void set_value(int x, int y, T value, T iso_value) {
		int i = y*resx+x;
		values[i] = value;
		flags[i] = value>iso_value;
	}
	///
	const int& index(int x, int y) const { return indices[y*resx+x]; }
			int& index(int x, int y)		 { return indices[y*resx+x]; }
	///
	const qem_type& get_qem(int x, int y) const { return cell_infos[y*resx+x].Q; }
			qem_type& ref_qem(int x, int y)		 { return cell_infos[y*resx+x].Q; }
	///
	int  count(int x, int y) const { return cell_infos[y*resx+x].count; }
	int& count(int x, int y)       { return cell_infos[y*resx+x].count; }
	///
	const pnt_type& center(int x, int y) const { return cell_infos[y*resx+x].center; }
	      pnt_type& center(int x, int y)       { return cell_infos[y*resx+x].center; }
	///
	const cell_info<T>& info(int x, int y) const { return cell_infos[y*resx+x]; }
	      cell_info<T>& info(int x, int y)       { return cell_infos[y*resx+x]; }
};

/// class used to perform the marching cubes algorithm
template <typename X, typename T>
class dual_contouring : public streaming_mesh<X>
{
public:
	typedef streaming_mesh<X> base_type;
	/// points must have three components
	typedef cgv::math::fvec<X,3> pnt_type;
	/// vectors must have three components
	typedef cgv::math::fvec<X,3> vec_type;
	/// qem type must have dimension three 
	typedef cgv::math::qem<X> qem_type;
	/// qem type must have dimension three 
	typedef cell_info<X> cell_info_type;
private:
	pnt_type p, minp;
	unsigned int resx, resy, resz;
	vec_type d;
	T iso_value;
protected:
	const cgv::math::v3_func<X,T>& func;
	X epsilon;
	unsigned int max_nr_iters;
	X consistency_threshold;
public:
	/// construct dual contouring object
	dual_contouring(const cgv::math::v3_func<X,T>& _func,
				    streaming_mesh_callback_handler* _smcbh, 
					const X& _consistency_threshold = 0.01f, unsigned int _max_nr_iters = 10,
					const X& _epsilon = 1e-6f) :
	func(_func), max_nr_iters(_max_nr_iters), consistency_threshold(_consistency_threshold), epsilon(_epsilon)
	{
		base_type::set_callback_handler(_smcbh);
	}
	/// construct a new vertex on an edge
	void compute_cell_vertex(dc_slice_info<T> *info_ptr, int i, int j)
	{
		if (info_ptr->count(i,j) == 0) {
			info_ptr->index(i,j) = -1;
			return;
		}
		pnt_type p_ref = info_ptr->center(i,j) / X(info_ptr->count(i,j));
		cgv::math::vec<X> min_pnt = info_ptr->get_qem(i, j).minarg(p_ref.to_vec(), X(0.1), d.length());
		pnt_type q(min_pnt.size(), min_pnt);
		info_ptr->index(i,j) = this->new_vertex(q);
	}
	/// construct a quadrilateral
	void generate_quad(unsigned int vi, unsigned int vj, unsigned int vk, unsigned int vl, bool reorient)
	{
		if (reorient)
			base_type::new_quad(vi,vl,vk,vj);
		else
			base_type::new_quad(vi,vj,vk,vl);
	}
	/// 
	void compute_edge_point(const T& _v_1, const T& _v_2, int e, const pnt_type& p, const vec_type& d,
		                     pnt_type& q, vec_type& n)
	{
		X de = d(e);
		T v_1 = _v_1;
		T v_2 = _v_2;
		pnt_type p_end = p;
		q = p;
		unsigned int nr_iters = 0;
		bool finished;
		X ref = consistency_threshold*fabs(v_2-v_1);
		do {
			finished = false;
			// compute point on edge
			X alpha;
			if (fabs(v_2-v_1) > epsilon) {
				finished = true;
				alpha = (X)(iso_value-v_1)/(v_2-v_1);
			}
			else 
				alpha = (X) 0.5;

			q(e) = p_end(e) - (1-alpha)*de;
			// compute normal at point
			cgv::math::vec<X> nml_vec = func.evaluate_gradient(q.to_vec());
			n = vec_type(nml_vec.size(), nml_vec);
			n.normalize();

			// stop if maximum number of iterations reached
			if (++nr_iters >= max_nr_iters)
				break; 
			// check if gradient is in accordance with value difference
			if (finished) {
				if (alpha < 0 || alpha > 1) {
					std::cout << "invalid alpha = " << alpha << std::endl;
				}
				X f_e = (v_2 - v_1) / de;
				if (fabs(f_e - n(e)) > consistency_threshold*f_e) {
					X v = func.evaluate(q.to_vec());
					if (fabs(v-iso_value) > ref) {
						if ((v > iso_value) == (v_2 > iso_value)) {
							de *= alpha;
							v_2 = v;
							p_end(e) = q(e);
						}
						else {
							v_1 = v;
							de *= 1-alpha;
						}
						finished = false;
					}
				}
			}
		} while (!finished);
		if (nr_iters > 15)
			std::cout << "not converged after " << nr_iters << " iterations" << std::endl;
		// if gradient does not define normal
		if (n.length() < 1e-6) {
			// define normal from edge direction
			n = vec_type(0, 0, 0);
			n(e) = T((v_1 < v_2) ? 1 : 0);
		}
		else 
			n.normalize();
	}
	/// construct plane through edge and add it to the incident qems
	void process_edge_plane(const T& v_1, const T& v_2, int e, 
									cell_info_type* C1, cell_info_type* C2, cell_info_type* C3, cell_info_type* C4)
	{
		pnt_type q;
		vec_type n;
		compute_edge_point(v_1, v_2, e, p, d, q, n);
		// construct qem
		qem_type Q(q.to_vec(),n.to_vec());
		// add qem to incident cells
		if (C1) { if (C1->count == 0) { C1->Q = Q; C1->center = q; } else { C1->Q += Q; C1->center += q; } ++C1->count; }
		if (C2) { if (C2->count == 0) { C2->Q = Q; C2->center = q; } else { C2->Q += Q; C2->center += q; } ++C2->count; }
		if (C3) { if (C3->count == 0) { C3->Q = Q; C3->center = q; } else { C3->Q += Q; C3->center += q; } ++C3->count; }
		if (C4) { if (C4->count == 0) { C4->Q = Q; C4->center = q; } else { C4->Q += Q; C4->center += q; } ++C4->count; }
	}
	/// process a slice
	void process_slice(dc_slice_info<T> *prev_info_ptr, dc_slice_info<T> *info_ptr)
	{
		unsigned int i,j;
		info_ptr->init();
		for (j = 0, p(1) = minp(1); j < resy; ++j, p(1) += d(1))
			for (i = 0, p(0) = minp(0); i < resx; ++i, p(0)+=d(0)) {
				// eval function on slice
				info_ptr->set_value(i,j,func.evaluate(p.to_vec()),iso_value);
				// process slice internal edges
				if (i > 0 && info_ptr->flag(i-1,j) != info_ptr->flag(i,j))
					process_edge_plane(info_ptr->value(i-1,j),
											 info_ptr->value(i,j), 0,
											 &info_ptr->info(i-1,j),
											 j > 0 ? &info_ptr->info(i-1,j-1) : 0,
											 prev_info_ptr ? &prev_info_ptr->info(i-1,j) : 0,
											 (j > 0 && prev_info_ptr) ? &prev_info_ptr->info(i-1,j-1) : 0);
				if (j > 0 && info_ptr->flag(i,j-1) != info_ptr->flag(i,j))
					process_edge_plane(info_ptr->value(i,j-1),
											 info_ptr->value(i,j), 1,
											 &info_ptr->info(i,j-1),
											 i > 0 ? &info_ptr->info(i-1,j-1) : 0,
											 prev_info_ptr ? &prev_info_ptr->info(i,j-1) : 0,
											 (i > 0 && prev_info_ptr) ? &prev_info_ptr->info(i-1,j-1) : 0);
			}
	}
	/// 
	void process_slab(dc_slice_info<T> *info_ptr_1, dc_slice_info<T> *info_ptr_2)
	{
		unsigned int i,j;
		for (j = 0, p(1) = minp(1); j < resy; ++j, p(1) += d(1))
			for (i = 0, p(0) = minp(0); i < resx; ++i, p(0)+=d(0)) {
				// process slab edges
				if (info_ptr_1->flag(i,j) != info_ptr_2->flag(i,j))
					process_edge_plane(info_ptr_1->value(i,j),
											 info_ptr_2->value(i,j), 2,
											 &info_ptr_1->info(i,j),
											 j > 0 ? &info_ptr_1->info(i,j-1) : 0,
											 i > 0 ? &info_ptr_1->info(i-1,j) : 0,
											 (i > 0 && j > 0) ? &info_ptr_1->info(i-1,j-1) : 0);
				// compute cell vertices
				if (i>0 && j>0)
					compute_cell_vertex(info_ptr_1, i-1,j-1);
			}
		// generate the quads of inner edges inside the slab
		for (j = 1, p(1) = minp(1)+d(1); j < resy-1; ++j, p(1) += d(1))
			for (i = 1, p(0) = minp(0)+d(0); i < resx-1; ++i, p(0)+=d(0))
				if (info_ptr_1->flag(i,j) != info_ptr_2->flag(i,j))
					generate_quad(info_ptr_1->index(i,j),
									  info_ptr_1->index(i-1,j),
									  info_ptr_1->index(i-1,j-1),
									  info_ptr_1->index(i,j-1),
									  info_ptr_1->flag(i,j));
	}
	/// 
	void generate_slice_quads(dc_slice_info<T> *info_ptr_1, dc_slice_info<T> *info_ptr_2)
	{
		unsigned int i,j;
		for (j = 1, p(1) = minp(1)+d(1); j < resy-1; ++j, p(1) += d(1))
			for (i = 1, p(0) = minp(0)+d(0); i < resx-1; ++i, p(0)+=d(0)) {
				if (info_ptr_2->flag(i-1,j) != info_ptr_2->flag(i,j))
					generate_quad(info_ptr_1->index(i-1,j),
									  info_ptr_2->index(i-1,j),
									  info_ptr_2->index(i-1,j-1),
									  info_ptr_1->index(i-1,j-1),
									  info_ptr_2->flag(i-1,j));
				if (info_ptr_2->flag(i,j-1) != info_ptr_2->flag(i,j))
					generate_quad(info_ptr_1->index(i,j-1),
									  info_ptr_1->index(i-1,j-1),
									  info_ptr_2->index(i-1,j-1),
									  info_ptr_2->index(i,j-1),
									  info_ptr_2->flag(i,j-1));
			}
	}
	/// extract iso surface and send quads to dual contouring handler
	void extract(const T& _iso_value,
				 const axis_aligned_box<X,3>& box,
				 unsigned int _resx, unsigned int _resy, unsigned int _resz,
				 bool show_progress = false)
	{
		// prepare private members
		resx = _resx; resy = _resy; resz = _resz;
		minp = p = box.get_min_pnt();
		d = box.get_extent();
		d(0) /= (resx-1); d(1) /= (resy-1); d(2) /= (resz-1);
		iso_value = _iso_value;

		// prepare progression
		cgv::utils::progression prog;
		if (show_progress) prog.init("extraction", resz, 10);

		// construct three slice infos
		dc_slice_info<T> slice_info_1(resx,resy), slice_info_2(resx,resy), slice_info_3(resx,resy);
		dc_slice_info<T> *slice_info_ptrs[3] = { &slice_info_1, &slice_info_2, &slice_info_3 };

		// iterate through all slices
		unsigned int nr_vertices[4] = { 0, 0, 0, 0 };
		unsigned int k, n;

		process_slice(0, slice_info_ptrs[0]);
		p(2) += d(2);
		process_slice(slice_info_ptrs[0], slice_info_ptrs[1]);
		process_slab(slice_info_ptrs[0], slice_info_ptrs[1]);
		p(2) += d(2);
		// show progression
		if (show_progress) {
			prog.step();
			prog.step();
		}
		for (k=2; k<resz; ++k, p(2) += d(2)) {
			n = base_type::get_nr_vertices();
			// evaluate function on next slice and construct slice interior vertices
			dc_slice_info<T> *info_ptr_0 = slice_info_ptrs[(k-2)%3];
			dc_slice_info<T> *info_ptr_1 = slice_info_ptrs[(k-1)%3];
			dc_slice_info<T> *info_ptr_2 = slice_info_ptrs[k%3];
			process_slice(info_ptr_1, info_ptr_2);
			process_slab(info_ptr_1, info_ptr_2);
			generate_slice_quads(info_ptr_0, info_ptr_1);

			n = base_type::get_nr_vertices()-n;
			nr_vertices[k%4] = n;
			n = nr_vertices[(k+3)%4];
			if (n > 0)
				base_type::drop_vertices(n);
			// show progression
			if (show_progress)
				prog.step();
		}
	}
};
		}
	}
}
