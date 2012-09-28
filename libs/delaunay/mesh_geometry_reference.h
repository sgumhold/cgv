#pragma once

#include "mesh_geometry.h"
#include <vector>

#include "lib_begin.h"

/// simple triangle mesh data structure using the corner data structure to store neighbor relations
template <typename ta_coord_type, class ta_point_type = point_2d<ta_coord_type> >
class CGV_API mesh_geometry_reference
{
public:
	/// declare coordinate type
	typedef ta_coord_type coord_type;
	/// declare point type
	typedef ta_point_type point_type;
	///
	typedef mesh_geometry<coord_type, point_type> referenced_type;
protected:
	/// list of points
	referenced_type* G;
public:
	/**@name construction*/
	//@{
	/// construct empty reference to geometry
	mesh_geometry_reference() : G(0) {}
	/// set the referenced geometry
	void set_reference_geometry(referenced_type* _G) { G = _G; }
	/// remove all vertices 
	void clear_geometry() { G->clear_geometry(); }
	/// return the number of vertices
	unsigned int get_nr_vertices() const { return G->get_nr_vertices(); }
	/// return a reference to the vertex location 
	point_type& p_of_vi(unsigned int vi) { return G->p_of_vi(vi); }
	/// return the location of a vertex
	const point_type& p_of_vi(unsigned int vi) const { return G->p_of_vi(vi); }
	/// add a new point to the geometry
	void add_point(const point_type& p) { return G->add_point(p); }
	/// check if point p is outside with respect to the line through p1 and p2
	static bool is_outside(const point_type& p, const point_type& p1, const point_type& p2) { return referenced_type::is_outside(p,p1,p2); }
	/// check if point p is inside with respect to the line through p1 and p2
	static bool is_inside(const point_type& p, const point_type& p1, const point_type& p2) { return referenced_type::is_inside(p,p1,p2); }
	/// check if point p is incident to the segment from p1 to p2
	static bool is_incident(const point_type& p, const point_type& p1, const point_type& p2) { return referenced_type::is_incident(p,p1,p2); }
	/// check if point p projects inside the segment from p1 to p2
	static bool is_between(const point_type& p, const point_type& p1, const point_type& p2) { return referenced_type::is_between(p,p1,p2); }
	/// compute the squared distance between two points
	static coord_type sqr_dist(const point_type& p0, const point_type& p1) { return referenced_type::sqr_dist(p0,p1); }
	/// compute the circum center of a triangle through p0, p1 and p2
	static point_type compute_circum_center(const point_type& p0, const point_type& p1, const point_type& p2) { return referenced_type::compute_circum_center(p0,p1,p2); }
};

#include <cgv/config/lib_end.h>