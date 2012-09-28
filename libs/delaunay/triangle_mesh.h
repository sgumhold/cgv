#pragma once

#include "mesh_geometry.h"
#include "ext_corner_connectivity.h"

#include "lib_begin.h"

/// simple triangle mesh data structure using the corner data structure to store neighbor relations
template <class ta_geometry = mesh_geometry<double>, class ta_connectivity = ext_corner_connectivity>
class CGV_API triangle_mesh : public ta_geometry, public ta_connectivity
{
public:
	/// type of geometry
	typedef ta_geometry     geometry_type;
	/// type of connectivity
	typedef ta_connectivity connectivity_type;
	///
	typedef typename connectivity_type::corner corner;
	///
	typedef typename geometry_type::point_type point_type;
	/** information resulting from a point location query. Either
	    the point is outside of the triangulation. In this case the
		 corner index specifies a corner opposite to a border edge
		 that has the point outside of the line passing through the
		 border edge. If the point is inside the triangulation ci is
		 one corner of the triangle containing the point. */
	struct point_location_info
	{
		/// whether the point is outside of the triangulation
		bool is_outside : 1;
		/// corner index of border edge opposite corner or triangle corner that contains point
		unsigned int ci : 31;
	};
	/// this structure is returned from the vertex insertion method
	struct vertex_insertion_info
	{
		/// whether the vertex could be inserted, or not
		bool insert_error : 1;
		/// whether a vertex with the same location already existed
		bool is_duplicate : 1;
		/// whether the new vertex extends the border of the triangulation
		bool extends_border : 1;
		/// index of a corner incident to the inserted vertex or its duplicate in the triangulation
		unsigned int ci_of_vertex;
	};
	/// the different hit types during tracing a segment through the triangulation
	enum TraceHitType { INCIDENT_VERTEX, INCIDENT_EDGE, INTERSECTED_EDGE };
	/// the trace hit structure describes one hit of the segment with the triangulation
	struct trace_hit
	{
		/// type of hit
		TraceHitType type;
		/// union of the index that describes the hit mesh element
		union {
			unsigned int vi_of_incident_vertex;
			unsigned int ci_of_incident_edge;
			unsigned int ci_of_intersected_edge;
		};
		/// constructor
		trace_hit(TraceHitType _type = INCIDENT_VERTEX, unsigned int idx = -1) : type(_type), ci_of_incident_edge(idx) {}
	};
public:
	/**@name construction*/
	//@{
	/// construct empty triangle mesh
	triangle_mesh();
	/// virtual destructor to allow for heap clean up
	virtual ~triangle_mesh();
	/// remove all vertices and triangles
	virtual void clear();
	/// add a triangle with all edges boundary edges, ensure correct orientation
	void add_triangle(unsigned int v0, unsigned int v1, unsigned int v2);
	/// add a triangle with the given cornern information, ensure correct orientation
	void add_triangle(const corner& c0, const corner& c1, const corner& c2);
	/// build a vertex fan on the border edge with corner index ci to the vertex vi and extend to all neighboring border edges to that vi is outside
	virtual void build_convex_fan_on_border(unsigned int ci, unsigned int vi);
	/// vertex insertion by vertex localization and simple 1 to 3 split of containing triangle or building of vertex fan
	virtual vertex_insertion_info insert_vertex(unsigned int vi, unsigned int start_ci = 0);
	/// compute triangulation with the help of the add triangle and vertex insertion method
	virtual void compute_triangulation();

	/// collect the mesh elements that are incident to the segment between vi and vj or the edges that intersect this segment
	void trace_segment(unsigned int vi, unsigned int vj, std::vector<trace_hit>& trace_hits) const;
	/// flip the edges that intersect segment between vi and vj such that (vi,vj) becomes an edge of the triangulation
	void flip_edges_to_insert_segment(unsigned int vi, unsigned int vj, std::vector<unsigned int>& intersected_edge_cis);
	//@}

	/**@name geometric predicates*/
	//@{
	/// extend flipable predicate by geometric considerations
	bool is_flipable(unsigned int c0) const;
	/// check if point p is outside with respect to the line through the edge opposite to the given corner
	bool is_outside(const point_type& p, unsigned int c0) const;
	/// check if point p is inside with respect to the line through the edge opposite to the given corner
	bool is_inside(const point_type& p, unsigned int c0) const;
	/// find the triangle containing p or the corner opposite to a border edge outside of which the p lies
	virtual point_location_info localize_point(const point_type& p, unsigned int start_ci = 0) const;
	/// compute the circum center of the triangle containing corner ci
	point_type compute_circum_center(unsigned int ci) const;
	///
	bool is_edge_flip_valid(unsigned int ci) const;
	//@}
};

#include <cgv/config/lib_end.h>