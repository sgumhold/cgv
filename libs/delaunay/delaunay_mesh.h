#pragma once

#include "mesh_geometry_reference.h"
#include "triangle_mesh.h"

#include "lib_begin.h"

/// simple triangle mesh data structure using the corner data structure to store neighbor relations
template <class ta_triangle_mesh = triangle_mesh<> >
class CGV_API delaunay_mesh : public ta_triangle_mesh
{
public:
	/// type of triangle mesh
	typedef ta_triangle_mesh triangle_mesh_type;
	///
	typedef typename triangle_mesh_type::corner corner;
	///
	typedef typename triangle_mesh_type::point_type point_type;
	///
	typedef typename triangle_mesh_type::coord_type coord_type;
	///
	typedef typename triangle_mesh_type::vertex_insertion_info vertex_insertion_info;
public:
	/**@name construction*/
	//@{
	/// construct empty delaunay mesh
	delaunay_mesh();
	//@}

	/**@name geometric predicates*/
	//@{
	/// return index of the nearest neighbor of the given point
	virtual unsigned int find_nearest_neighbor(const point_type& p, unsigned int ci_start = 0) const;
	/// check if the edge opposite to the given corner is locally delaunay
	bool is_locally_delaunay(unsigned int ci) const;
	//@}

	/**@name delaunay*/
	//@{
	/// flip the edges in the one ring of vertex with given corner until all are valid meaning locally delaunay
	virtual void flip_edges_around_vertex_to_validate(unsigned int ci, unsigned int n, std::vector<unsigned int>* touched_corners = 0);
	/// reimplement vertex insertion in order to keep a delaunay triangulation. If a vertex with the same location already exists, ignore vertex and return index of vertex with identical location
	vertex_insertion_info insert_vertex(unsigned int vi, unsigned int ci_start = 0, std::vector<unsigned int>* touched_corners = 0);
	//@}
};

#include <cgv/config/lib_end.h>