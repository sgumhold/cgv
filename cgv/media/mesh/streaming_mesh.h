#pragma once

#include <vector>
#include <deque>
#include <cgv/math/fvec.h>

namespace cgv {
	namespace media {
		namespace mesh {

/** pure abstract interface to handle callbacks of a streaming mesh */
struct streaming_mesh_callback_handler
{
	/// called when a new vertex is generated
	virtual void new_vertex(unsigned int vertex_index) = 0;
	/// announces a new polygon defines by the vertex indices stored in the given vector
	virtual void new_polygon(const std::vector<unsigned int>& vertex_indices) = 0;
	/// drop the currently first vertex that has the given global vertex index
	virtual void before_drop_vertex(unsigned int vertex_index) = 0;
};

/// class used to perform the marching cubes algorithm
template <typename T>
class streaming_mesh
{
public:
	/// type of vertex locations
	typedef cgv::math::fvec<T,3> pnt_type;
	/// type of vertex normals
	typedef cgv::math::fvec<T,3> vec_type;
protected:
	/// offset used to address vertices in deque
	int idx_off;
	/// count the number of faces
	unsigned int nr_faces;
	/// store currently used points in deque
	std::deque<pnt_type> pnts;
	/// store currently used normals in deque
	std::deque<vec_type> nmls;
	/// store a pointer to the callback handler
	streaming_mesh_callback_handler* smcbh;
public:
	/// construct from callback handler
	streaming_mesh(streaming_mesh_callback_handler* _smcbh = 0) : smcbh(_smcbh), nr_faces(0), idx_off(0) {
	}
	/// set a new callback handler
	void set_callback_handler(streaming_mesh_callback_handler* _smcbh) {
		smcbh = _smcbh;
	}
	/// return the number of vertices dropped from the front, what is used as index offset into a deque
	unsigned int get_nr_dropped_vertices() const           { return idx_off; }
	/// return the number of vertices
	unsigned int get_nr_vertices() const                   { return (unsigned int) pnts.size()+idx_off; }
	/// return the number of faces
	unsigned int get_nr_faces() const                      { return nr_faces; }
	/// drop the front most vertex from the deque
	void drop_vertex() {
		if (pnts.empty())
			return;
		if (smcbh)
			smcbh->before_drop_vertex(idx_off);
		pnts.pop_front();
		nmls.pop_front();
		++idx_off;
	}
	/// drop n vertices from the front of the deque
	void drop_vertices(unsigned int n) {
		for (unsigned int i=0; i<n; ++i)
			drop_vertex();
	}
	/// write access to vertex locations
		   pnt_type& vertex_location(unsigned int vi)       { return pnts[vi-idx_off]; }
	/// read access to vertex locations
	const pnt_type& vertex_location(unsigned int vi) const { return pnts[vi-idx_off]; }
	/// read access to vertex normals
	const vec_type& vertex_normal(unsigned int vi) const   { return nmls[vi-idx_off]; }
	/// write access to vertex normals
         vec_type& vertex_normal(unsigned int vi)         { return nmls[vi-idx_off]; }
	/// add a new vertex with the given location and call the callback of the callback handler
	unsigned int new_vertex(const pnt_type& p) {
		unsigned int vi = (int)pnts.size()+idx_off;
		pnts.push_back(p);
		nmls.push_back(vec_type(0,0,0));
		if (smcbh)
			smcbh->new_vertex(vi);
		return vi;
	}
	/// construct a new triangle by calling the new polygon method of the callback handler
	void new_triangle(unsigned int vi, unsigned int vj, unsigned int vk) {
		static std::vector<unsigned int> vis(3);
		vis[0] = vi;
		vis[1] = vj;
		vis[2] = vk;
		++nr_faces;
		if (smcbh)
			smcbh->new_polygon(vis);
	}
	/// construct a new quad by calling the new polygon method of the callback handler
	void new_quad(unsigned int vi, unsigned int vj, unsigned int vk, unsigned int vl) {
		static std::vector<unsigned int> vis(4);
		vis[0] = vi;
		vis[1] = vj;
		vis[2] = vk;
		vis[3] = vl;
		++nr_faces;
		if (smcbh)
			smcbh->new_polygon(vis);
	}
	/// construct a new polygon by calling the new polygon method of the callback handler
	void new_polygon(const std::vector<unsigned int>& vertex_indices) {
		++nr_faces;
		if (smcbh)
			smcbh->new_polygon(vertex_indices);
	}
};

		}
	}
}