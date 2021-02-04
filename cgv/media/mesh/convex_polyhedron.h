#pragma once

#include <map>
#include <iterator> 
#include <cgv/math/fvec.h>
#include <cgv/media/axis_aligned_box.h>
#include <cgv/media/color.h>

#include "../lib_begin.h"

namespace cgv {
	namespace media {
		namespace mesh {

class CGV_API convex_polyhedron_base
{
public:
	/// enumeration of different vertex locations with respect to plane
	enum VertexPlaneLocation {
		VPL_INSIDE = -1,
		VPL_TOUCH = 0,
		VPL_OUTSIDE = 1
	};
	/// enumeration of 11 valid and 5 invalid cases that a face can be located with respect to a plane after locating vertices into outside, inside or touching
	enum FacePlaneLocation {
		// valid cases
		FPL_OUTSIDE = 0,                    // face is outside (is refined by TOUCH-versions by adding or or-ring together)
		FPL_INSIDE = 3,                     // face is inside (is refined by TOUCH-versions by adding or or-ring together)

		FPL_TOUCH_NOTHING = 0,              // face is not touching and is not split but completely outside or inside   (v#+|-=0, v#0=0)[e#0=0, e#s=0, #t=0]
		FPL_TOUCH_VERTEX = 1,               // one vertex of face touches plane                                         (v#+|-=0, v#0=1)[e#0=0, e#s=0, #t=0]
		FPL_TOUCH_EDGE = 2,                 // one edge of face touches plane                                           (v#+|-=0, v#0=2, e#0=1)[e#s=0, #t=0]

		FPL_TOUCH_FACE = 6,                 // whole face touches plane                                                 (v#+|-=0, v#0=n)[e#0=0, e#s=0, #t=0]

		FPL_SPLIT_TWO_VERTICES,             // face is split through two not adjacent vertices                          (#t=2, v#0=2, e#s=0)[e#0=0, v#+>0, v#->0]
		FPL_SPLIT_VERTEX_EDGE,              // face is split through one vertex and one edge                            (#t=2, v#0=1, e#s=1)[e#0=0, v#+>0, v#->0]
		FPL_SPLIT_TWO_EDGES,                // face is split through two edges                                          (#t=2, v#0=0, e#s=2)[e#0=0, v#+>0, v#->0]

		// causes of invalidity
		FPL_TWO_NON_ADJACENT_VERTICES_TOUCH,// two non adjacent vertices touch plane                                    (v#+|-=0, v#0=2, e#0=0)
		FPL_INCOMLETE_TOUCH_FACE,           // more than two and less than all vertices touch plane                     (v#0>2, v#0<n)
		FPL_TOUCH_AND_SPLIT,                // in addition to a split we have further touch events                      (#t=2, v#0+e#s>2)
		FPL_MULTI_SPLIT,                    // more than one pair of inside->outside, outside->inside transitions found (#t>2)
		FPL_ODD_TRANSITION_NUMBER,          // the computed number of transitions is odd                                (#t%2=1)

		FPL_UNCONSIDERED_CASE,

		FPL_FIRST_INVALID_CASE = FPL_TWO_NON_ADJACENT_VERTICES_TOUCH
	};

	/// a face is an oriented list of vertex indices
	typedef std::vector<int> face_type;

	/// a shell is simply a list of faces
	typedef std::vector<face_type> shell_type;

protected:
	/// vector of shells
	std::vector<shell_type> shells;
public:
	/// convert \c VertexPlaneLocation to an index in the range {0 .. 2}
	static int to_index(VertexPlaneLocation vertex_location) { return (int)vertex_location + 1; }
	/// check if a face location is on one side of the plane only with potential touch events
	static bool is_face_location_on_one_side(FacePlaneLocation face_location) { return face_location < FPL_TOUCH_FACE; }
	/// check if a face location corresponds to a split into parts (two for convex faces)
	static bool is_face_location_split(FacePlaneLocation face_location) { return face_location > FPL_TOUCH_FACE && face_location < FPL_FIRST_INVALID_CASE; }
	/// check if a face location corresponds to a valid configuration assuming planar convex faces
	static bool is_valid_face_location(FacePlaneLocation face_location) { return face_location < FPL_FIRST_INVALID_CASE; }
	/// function implementing the connectivity of an axis aligned box
	static int  box_get_face_vertex_index(int fi, int ci);
	/// revert the orientation of a face in place, where the face is given by a list of vertex indices 
	static void change_orientation(face_type& face);
	/// check whether all vertices lay on one side of the plane or on it, but not on the other; return side or VPL_TOUCH if vertices lay on both sides
	static VertexPlaneLocation vertex_location_side(const std::vector<VertexPlaneLocation>& vertex_locations);
	/// check whether all faces of a shell lay on one side of the plane or touch it, but not on the other side; return side or VPL_TOUCH if shell is intersected by plane
	static VertexPlaneLocation shell_location_side(const std::vector<FacePlaneLocation>& face_locations);

	/// given a face as a list of vertex indices and a vector of vertex locations, find a halfedge that touches the current clip plane
	static std::pair<int, int> extract_touching_halfedge(const face_type& face, const std::vector<VertexPlaneLocation>& vertex_locations);
	/// given a face as a list of vertex indices and a vector of vertex locations, determine the face location with respect to the current clip plane
	FacePlaneLocation get_face_plane_location(const face_type& face, const std::vector<VertexPlaneLocation>& vertex_locations) const;
	/**@name handling of shells and faces*/
	//@{
	/// return the current number of shells
	size_t get_nr_shells() const { return shells.size(); }
	///
	const shell_type&  shell(int si) const { return shells[si]; }
	///
	shell_type&  shell(int si)       { return shells[si]; }
	/// add a new shell and return shell index
	virtual int add_shell() { shells.push_back(shell_type()); return (int)shells.size() - 1; }
	/// remove the shell of the given index, be careful as shell indices large si get implizitely decreased
	virtual void del_shell(int si) { shells.erase(shells.begin() + si); }
	/// copy a shell onto another one
	virtual void copy_shell(int si_source, int si_target) { shell(si_target) = shell(si_source); }
	/// swap two shells
	virtual void swap_shells(int si0, int si1) { std::swap(shell(si0), shell(si1)); }
	/// 
	size_t get_nr_faces(int si = 0) const { return shell(si).size(); }
	///
	virtual int add_face(int si = 0) { shell(si).push_back(face_type()); return (int)shell(si).size() - 1; }
	/// 
	virtual void del_face(int fi, int si = 0) { shell(si).erase(shell(si).begin() + fi); }
	///
	const face_type&  face(int fi, int si = 0) const { return shell(si)[fi]; }
	///
	face_type&  face(int fi, int si = 0) { return shell(si)[fi]; }
	//@}
	/**@name handling of vertices*/
	//@{
	/// return current number of vertices
	virtual size_t get_nr_vertices() const = 0;
	/// virtual method to be implemented in derived class 
	virtual void del_vertex(int vi) = 0;
	/// remove redundant vertices from mesh
	void remove_redundant_vertices();
	//@}
};

//! simple mesh templated with per vertex position and texture coordinates and per face plane.
/*! First template parameter is common type and the second is the dimension of texture coordinates */
template <typename T, cgv::type::int32_type TCDim = 2>
class convex_polyhedron : public convex_polyhedron_base
{
public:
	/// declare coordinate type
	typedef T coord_type;
	/// declare dimension of texture coordinates
	const cgv::type::int32_type texcoord_dim = TCDim;
	/// position type
	typedef typename cgv::math::fvec<T, 3> point_type;
	/// texture coordinate type
	typedef typename cgv::math::fvec<T, TCDim> texcoord_type;
	/// plane type
	typedef typename cgv::math::fvec<T, 4> plane_type;
	/// type of axis aligned box
	typedef typename cgv::media::axis_aligned_box<T, 3> box_type;
	/// type of a vertex
	struct vertex_type
	{
		point_type position;
		texcoord_type texcoord;
	};
protected:
	/// vector to store vertices of all components in one vector
	std::vector<vertex_type> vertices;
	/// per shell a vector to store one plane per face
	std::vector<std::vector<plane_type> > face_planes;
public:
	/**@name vertex handling */
	//@{
	/// 
	size_t get_nr_vertices() const { return vertices.size(); }
	///
	virtual int add_vertex() { vertices.push_back(vertex_type()); return (int)get_nr_vertices() - 1; }
	/// remove a vertex
	void del_vertex(int vi) { vertices.erase(vertices.begin() + vi); }
	///
	const vertex_type& vertex(int vi) const { return vertices[vi]; }
	///
	vertex_type& vertex(int vi) { return vertices[vi]; }
	///
	const point_type& position(int vi) const { return vertices[vi].position; }
	///
	point_type& position(int vi)       { return vertices[vi].position; }
	///
	const texcoord_type& texcoord(int vi) const { return vertices[vi].texcoord; }
	///
	texcoord_type& texcoord(int vi)       { return vertices[vi].texcoord; }
	//@}


	/**@name handling of face planes */
	//@{
	/// check if planes are oriented in the same normal direction
	static bool orientation_match(const plane_type& p, const plane_type& q) {
		return dot(reinterpret_cast<const point_type&>(p), reinterpret_cast<const point_type&>(q)) > 0;
	}
	/// return const reference to plane of face fi in shell si
	const plane_type& face_plane(int fi, int si = 0) const { return face_planes[si][fi]; }
	/// return reference to plane of face fi in shell si
	plane_type& face_plane(int fi, int si = 0) { return face_planes[si][fi]; }
	/// compute a face plane from the vertex positions
	plane_type compute_face_plane(int fi, int si = 0) const {
		point_type center(0, 0, 0);
		point_type normal(0, 0, 0);
		int last_vi = face(fi, si).back();
		for (int vi : face(fi, si)) {
			center += position(vi);
			normal += cross(position(last_vi), position(vi));
			last_vi = vi;
		}
		normal.normalize();
		center *= T(1)/face(fi, si).size();
		return plane_type(normal(0), normal(1), normal(2), -dot(center, normal));
	}
	/// add a new shell and return shell index
	int add_shell() {
		face_planes.push_back(std::vector<plane_type>());
		return convex_polyhedron_base::add_shell();
	}
	/// remove the shell of the given index, be careful as shell indices large si get implizitely decreased
	void del_shell(int si) {
		face_planes.erase(face_planes.begin() + si);
		convex_polyhedron_base::del_shell(si);
	}
	/// copy a shell onto another one
	void copy_shell(int si_source, int si_target) { 
		face_planes[si_target] = face_planes[si_source];
		convex_polyhedron_base::copy_shell(si_source, si_target);
	}
	/// swap two shells
	void swap_shells(int si0, int si1) { 
		std::swap(face_planes[si0], face_planes[si1]);
		convex_polyhedron_base::swap_shells(si0, si1);
	}
	///
	int add_face(int si = 0) {
		face_planes[si].push_back(plane_type());
		return convex_polyhedron_base::add_face(si);
	}
	/// 
	void del_face(int fi, int si = 0) { 
		face_planes[si].erase(face_planes[si].begin() + fi); 
		convex_polyhedron_base::del_face(fi, si);
	}
	//@}

	/// construct a polyhedron representing an axis aligned box
	void construct_box(const box_type& box);
	/// clear data structure
	void clear();
	/// construct a vector of vertex locations relative to given plane
	void compute_vertex_locations(const plane_type& plane, std::vector<VertexPlaneLocation>& vertex_locations, T epsilon = 16 * std::numeric_limits<T>::epsilon(), std::vector<T>* vertex_signed_distances_ptr = 0) const;
	/// compute face plane locations of the faces of one shell, store them in the vector \c face_locations and return if all is valid (assuming convex shape and convex faces)
	bool check_face_locations(unsigned si, const std::vector<VertexPlaneLocation>& vertex_locations, std::vector<FacePlaneLocation>& face_locations) const;
	/// if the previous check is invalid, call this function to change the vertex signs in order to validate the face plane locations (not implemented yet)
	bool ensure_vertex_location_consistency(unsigned si, const std::vector<T>& vertex_signed_distances, std::vector<VertexPlaneLocation>& vertex_locations, const std::vector<FacePlaneLocation>& face_locations, T epsilon = 16 * std::numeric_limits<T>::epsilon(), T epsilon_flexibiliy = 8);
	//! compute the interior of the given shell si with respect to the given clipping plane and either replace the shell or create a new one
	/*! return the index of the new shell or -1 if the shell was empty and therefore not created. If the current shell was to be replace with an empty result, it is
	simply deleted. */
	int clip_to_inside_of_plane(unsigned si, const plane_type& clip_plane, bool keep_original_shell = false, T epsilon = 16 * std::numeric_limits<T>::epsilon(), T epsilon_flexibiliy = 8);
	//! compute the exterior of the given shell si with respect to the given clipping plane and either replace the shell or create a new one
	/*! return the index of the new shell or -1 if the shell was empty and therefore not created. If the current shell was to be replace with an empty result, it is
	simply deleted. */
	int clip_to_outside_of_plane(unsigned si, const plane_type& clip_plane, bool keep_original_shell = false, T epsilon = 16 * std::numeric_limits<T>::epsilon(), T epsilon_flexibiliy = 8);
	//! split the given shell into interior and exterior
	/*! return shell index pair where first index corresponds to interior part and second to exterior part. 
	    If a shell is empty the returned shell index is set to -1.*/
	std::pair<int,int> split_at_plane(unsigned si, const plane_type& split_plane, bool keep_original_shell = false, T epsilon = 16 * std::numeric_limits<T>::epsilon(), T epsilon_flexibiliy = 8, std::vector<VertexPlaneLocation>* vertex_locations_ptr = 0);
	/// compute the intersection polygon between a shell and a plane
	std::vector<vertex_type> compute_intersection_polygon(unsigned si, const plane_type& plane, T epsilon = 16 * std::numeric_limits<T>::epsilon(), T epsilon_flexibiliy = 8) const;
};

/// construct a polyhedron representing an axis aligned box
template <typename T, cgv::type::int32_type TCDim>
void convex_polyhedron<T,TCDim>::construct_box(const box_type& box)
{
	int i, c;
	// construct corner vertices
	vertices.resize(8);
	for (i = 0; i < 8; ++i) {
		vertex_type &V = vertices[i];
		for (c = 0; c < 3; ++c) {
			V.position(c) = ((i & (1 << c)) == 0) ? box.get_min_pnt()(c) : box.get_max_pnt()(c);
			if (c < texcoord_dim)
				V.texcoord(c) = ((i & (1 << c)) == 0) ? T(0) : T(1);
		}
	}
	// construct node
	int si = add_shell();
	shell_type& S = shell(si);
	// construct faces
	S.clear();
	for (i = 0; i < 6; ++i) {
		int fi = add_face(si);
		plane_type& P = face_plane(fi,si);
		P.zeros();
		P(i / 2) = ((i & 1) == 0) ? T(1) : T(-1);
		P(3) = ((i & 1) == 0) ? -box.get_max_pnt()(i / 2) : box.get_min_pnt()(i / 2);
		face_type& F = face(fi, si);
		for (unsigned j = 0; j < 4; ++j)
			F.push_back(box_get_face_vertex_index(i, j));
	}
}

/// clear data structure
template <typename T, cgv::type::int32_type TCDim>
void convex_polyhedron<T, TCDim>::clear()
{
	vertices.clear();
	while (get_nr_shells() > 0)
		del_shell((int)get_nr_shells() - 1);
}

///
template <typename T, cgv::type::int32_type TCDim>
bool convex_polyhedron<T, TCDim>::check_face_locations(unsigned si, const std::vector<VertexPlaneLocation>& vertex_locations, std::vector<FacePlaneLocation>& face_locations) const
{
	bool valid = true;
	const shell_type& S = shell(si);
	unsigned nr_faces = (unsigned)get_nr_faces(si);
	unsigned nr_face_touches = 0;
	face_locations.resize(nr_faces);
	for (unsigned fi = 0; fi < nr_faces; ++fi) {
		const face_type& F = S[fi];
		face_locations[fi] = get_face_plane_location(face(fi, si), vertex_locations);
		if (face_locations[fi] == FPL_TOUCH_FACE)
			++nr_face_touches;
		if (!is_valid_face_location(face_locations[fi]))
			valid = false;
	}
	if (nr_face_touches > 1)
		valid = false;
	return valid;
}

///
template <typename T, cgv::type::int32_type TCDim>
bool convex_polyhedron<T, TCDim>::ensure_vertex_location_consistency(unsigned ni, const std::vector<T>& vertex_signed_distances, std::vector<VertexPlaneLocation>& vertex_locations, const std::vector<FacePlaneLocation>& face_locations, T epsilon, T epsilon_flexibiliy)
{
	return false;
}

/// compute the interior of the given shell si with respect to the given clipping plane and either replace the shell or create a new one
template <typename T, cgv::type::int32_type TCDim>
int convex_polyhedron<T, TCDim>::clip_to_inside_of_plane(unsigned si, const plane_type& clip_plane, bool keep_original_shell, T epsilon, T epsilon_flexibiliy)
{
	std::pair<int, int> res = split_at_plane(si, clip_plane, keep_original_shell, epsilon, epsilon_flexibiliy);
	if (res.second == -1)
		return res.first;
	if (res.first == -1)
		return -1;
	if (res.first < res.second) {
		del_shell(res.second);
		return res.first;
	}
	copy_shell(res.first, res.second);
	del_shell(res.first);
	return res.second;
}

/// compute the exterior of the given shell si with respect to the given clipping plane and either replace the shell or create a new one
template <typename T, cgv::type::int32_type TCDim>
int convex_polyhedron<T, TCDim>::clip_to_outside_of_plane(unsigned si, const plane_type& clip_plane, bool keep_original_shell, T epsilon, T epsilon_flexibiliy)
{
	std::pair<int, int> res = split_at_plane(si, clip_plane, keep_original_shell, epsilon, epsilon_flexibiliy);
	if (res.first == -1)
		return res.second;
	if (res.second == -1)
		return -1;
	if (res.second < res.first) {
		del_shell(res.first);
		return res.second;
	}
	copy_shell(res.second, res.first);
	del_shell(res.second);
	return res.first;
}


template <typename T, cgv::type::int32_type TCDim>
void convex_polyhedron<T, TCDim>::compute_vertex_locations(const plane_type& plane, std::vector<VertexPlaneLocation>& vertex_locations, T epsilon, std::vector<T>* vertex_signed_distances_ptr) const
{
	// compute signed distances of vertices from clipping plane and compute sign including 0
	unsigned vi;
	vertex_locations.resize(get_nr_vertices());
	std::vector<T> vertex_signed_distances_local;
	std::vector<T>& vertex_signed_distances = vertex_signed_distances_ptr ? *vertex_signed_distances_ptr : vertex_signed_distances_local;
	vertex_signed_distances.resize(get_nr_vertices());
	for (vi = 0; vi < get_nr_vertices(); ++vi) {
		T sd = dot(position(vi), reinterpret_cast<const point_type&>(plane)) + plane(3);
		vertex_signed_distances[vi] = sd;
		vertex_locations[vi] = (sd < -epsilon) ? VPL_INSIDE : ((sd > epsilon) ? VPL_OUTSIDE : VPL_TOUCH);
	}
}

///
template <typename T, cgv::type::int32_type TCDim>
std::pair<int, int> convex_polyhedron<T, TCDim>::split_at_plane(unsigned si, const plane_type& split_plane, bool keep_original_shell, T epsilon, T epsilon_flexibiliy, std::vector<VertexPlaneLocation>* vertex_locations_ptr)
{
	// first compute vertex locations
	std::vector<VertexPlaneLocation> vertex_locations_local;
	std::vector<VertexPlaneLocation>& vertex_locations = vertex_locations_ptr ? *vertex_locations_ptr : vertex_locations_local;
	std::vector<T> vertex_signed_distances;
	compute_vertex_locations(split_plane, vertex_locations, epsilon, &vertex_signed_distances);

	// next compute face locations and check for invalid cases
	std::vector<FacePlaneLocation> face_locations;
	if (!check_face_locations(si, vertex_locations, face_locations)) {
		if (!ensure_vertex_location_consistency(si, vertex_signed_distances, vertex_locations, face_locations)) {
			std::cerr << "found case that could not be made consistent" << std::endl;
			abort();
		}
	}

	// check if intersection is empty
	VertexPlaneLocation shell_location = shell_location_side(face_locations);
	// if complete shell is inside, no split computation is necessary
	if (shell_location == VPL_INSIDE) {
		// in case the original shell is not kept, reuse it as result for interior shell
		if (!keep_original_shell)
			return std::pair<int, int>(si, -1);
		// otherwise construct copy of shell
		int new_si = add_shell();
		copy_shell(si, new_si);
		return std::pair<int, int>(new_si, -1);
	}
	// if complete shell is outside
	if (shell_location == VPL_OUTSIDE) {
		// in case the original shell is not kept, reuse it as result for exterior shell
		if (!keep_original_shell)
			return std::pair<int, int>(-1, si);
		// otherwise construct copy of shell
		int new_si = add_shell();
		copy_shell(si, new_si);
		return std::pair<int, int>(-1, new_si);
	}

	// collect halfedges of face of new plane
	std::vector<std::pair<int, int> > new_face_halfedges;
	// use a map to hash the newly created vertices per edge
	std::map<std::pair<int, int>, int> edge_vertex_indices;
	// use a map to hash touched edges with their locations to allow adding edges of the new face
	std::map<std::pair<int, int>, FacePlaneLocation> edge_touch_events;

	// create two new shells
	int si_inside  = add_shell();
	int si_outside = add_shell();
	// go through all faces
	for (unsigned fi = 0; fi < get_nr_faces(si); ++fi) {
		switch (face_locations[fi]) {
		// outside faces that do not touch are only touch in vertex or edge are kept
		case FPL_OUTSIDE + FPL_TOUCH_EDGE:
		{
			// check touching edges for matching insides to also consider them for edges of the new face
			std::pair<int, int> halfedge = extract_touching_halfedge(face(fi,si), vertex_locations);
			std::pair<int, int> edge(std::min(halfedge.first, halfedge.second), std::max(halfedge.first, halfedge.second));
			auto ete_iter = edge_touch_events.find(edge);
			// if yes  
			if (ete_iter != edge_touch_events.end()) {
				// check for opposite side
				if (ete_iter->second == FPL_INSIDE + FPL_TOUCH_EDGE)
					new_face_halfedges.push_back(std::pair<int, int>(halfedge.second, halfedge.first));
				// erase matched and unmatched events
				edge_touch_events.erase(ete_iter);
			}
			// otherwise create new edge touch event
			else
				edge_touch_events[edge] = FacePlaneLocation(FPL_OUTSIDE + FPL_TOUCH_EDGE);
		}
		case FPL_OUTSIDE + FPL_TOUCH_NOTHING:
		case FPL_OUTSIDE + FPL_TOUCH_VERTEX:
		{
			int fi_outside = add_face(si_outside);
			face(fi_outside, si_outside) = face(fi, si);
			face_plane(fi_outside, si_outside) = face_plane(fi, si);
			break;
		}
			// inside faces that do not touch are only touch in vertex or edge are kept
		case FPL_INSIDE + FPL_TOUCH_EDGE:
		{
			// check touching edges for matching outsides to also consider them for edges of the new face
			std::pair<int, int> halfedge = extract_touching_halfedge(face(fi, si), vertex_locations);
			std::pair<int, int> edge(std::min(halfedge.first, halfedge.second), std::max(halfedge.first, halfedge.second));
			auto ete_iter = edge_touch_events.find(edge);
			// if yes  
			if (ete_iter != edge_touch_events.end()) {
				// check for opposite side
				if (ete_iter->second == FPL_OUTSIDE + FPL_TOUCH_EDGE)
					new_face_halfedges.push_back(halfedge);
				// erase matched and unmatched events
				edge_touch_events.erase(ete_iter);
			}
			// otherwise create new edge touch event
			else
				edge_touch_events[edge] = FacePlaneLocation(FPL_INSIDE + FPL_TOUCH_EDGE);
		}
		case FPL_INSIDE + FPL_TOUCH_NOTHING:
		case FPL_INSIDE + FPL_TOUCH_VERTEX:
		{
			int fi_inside = add_face(si_inside);
			face(fi_inside, si_inside) = face(fi, si);
			face_plane(fi_inside, si_inside) = face_plane(fi, si);
			break;
		}
		// completely touching faces are kept and dublicated 
		case FPL_TOUCH_FACE:
		{
			int fi_outside = add_face(si_outside);
			int fi_inside = add_face(si_inside);
			face(fi_outside, si_outside) = face(fi_inside, si_inside) = face(fi, si);
			face_plane(fi_outside, si_outside) = -face_plane(fi, si);
			face_plane(fi_inside, si_inside) = face_plane(fi, si);
			if (orientation_match(split_plane, face_plane(fi, si)))
				change_orientation(face(fi_outside, si_outside));
			else
				change_orientation(face(fi_inside, si_inside));
			break;
		}
		// in the split cases we need to iterate the halfedges
		case FPL_SPLIT_TWO_VERTICES:
		case FPL_SPLIT_VERTEX_EDGE:
		case FPL_SPLIT_TWO_EDGES:
		{
			const face_type& F = face(fi, si);
			// add an empty face on both sides
			int fi_outside = add_face(si_outside);
			int fi_inside = add_face(si_inside);
			face_type& F_outside = face(fi_outside, si_outside);
			face_type& F_inside = face(fi_inside, si_inside);
			// copy face plane to both sides
			face_plane(fi_outside, si_outside) = face_plane(fi_inside, si_inside) = face_plane(fi, si);
			// loop face vertex indices and update counts
			std::vector<int> split_vis;
			int last_vi = F.back();
			VertexPlaneLocation last_vertex_location = vertex_locations[last_vi];
			for (unsigned i = 0; i < F.size(); ++i) {
				int vi = F[i];
				VertexPlaneLocation vertex_location = vertex_locations[vi];
				// touching vertices go into both face loops
				if (vertex_location == VPL_TOUCH) {
					F_outside.push_back(vi);
					F_inside.push_back(vi);
					split_vis.push_back(vi);
					if (split_vis.size() == 2 && last_vertex_location == VPL_INSIDE)
						std::swap(split_vis[0], split_vis[1]);
				}
				// if previous halfedge is split, generate split vertex and add to both loops
				else {
					if (last_vertex_location != VPL_TOUCH && vertex_location != last_vertex_location) {
						// check if split vertex has already been created on inverse halfedge
						std::pair<int, int> edge(std::min(last_vi, vi), std::max(last_vi, vi));
						auto evi_iter = edge_vertex_indices.find(edge);
						int split_vi;
						// if yes, get vertex index and remove entry from map
						if (evi_iter != edge_vertex_indices.end()) {
							split_vi = evi_iter->second;
							edge_vertex_indices.erase(evi_iter);
						}
						// otherwise create new vertex on edge by affine interpolation according to unsigned distances
						else {
							split_vi = (int)vertices.size();
							T distance = abs(vertex_signed_distances[vi]);
							T last_distance = abs(vertex_signed_distances[last_vi]);
							T lambda = last_distance / (last_distance + distance);
							vertex_type split_vertex;
							split_vertex.position = (T(1) - lambda) * position(last_vi) + lambda * position(vi);
							split_vertex.texcoord = (T(1) - lambda) * texcoord(last_vi) + lambda * texcoord(vi);
							vertices.push_back(split_vertex);
							vertex_locations.push_back(VPL_TOUCH);
							edge_vertex_indices[edge] = split_vi;
						}
						split_vis.push_back(split_vi);
						if (split_vis.size() == 2 && last_vertex_location == VPL_INSIDE)
							std::swap(split_vis[0], split_vis[1]);

						// add split vertex to both faces
						F_outside.push_back(split_vi);
						F_inside.push_back(split_vi);
					}
					// then add current vertex only to current loop
					if (vertex_location == VPL_INSIDE)
						F_inside.push_back(vi);
					else
						F_outside.push_back(vi);
				}
				last_vi = vi;
				last_vertex_location = vertex_location;
			}
			if (split_vis.size() != 2) {
				std::cerr << "expected two split vertices but found " << split_vis.size() << std::endl;
				abort();
			}
			new_face_halfedges.push_back(std::pair<int, int>(split_vis[0], split_vis[1]));
			break;
		}
		default:
			std::cerr << "did not expect face plane location " << face_locations[fi] << std::endl;
			abort();
		}
	}
	// finally we need to add one face for the new plane on each side

	// prepare new faces
	int fi_outside = add_face(si_outside);
	int fi_inside = add_face(si_inside);
	face_type& F_outside = face(fi_outside, si_outside);
	face_type& F_inside = face(fi_inside, si_inside);
	face_plane(fi_outside, si_outside) = -split_plane;
	face_plane(fi_inside, si_inside) = split_plane;

	// first link the new face for the outside shell
	F_outside.resize(new_face_halfedges.size());
	F_outside[0] = new_face_halfedges[0].first;
	int last_vi = F_outside[1] = new_face_halfedges[0].second;
	unsigned k0 = 1;
	for (unsigned i = 2; i < F_outside.size(); ++i) {
		bool found = false;
		for (unsigned k = k0; k < new_face_halfedges.size(); ++k) {
			if (new_face_halfedges[k].first == last_vi) {
				last_vi = F_outside[i] = new_face_halfedges[k].second;
				if (k > k0)
					std::swap(new_face_halfedges[k0], new_face_halfedges[k]);
				++k0;
				found = true;
				break;
 			}
		}
		if (!found) {
			std::cerr << "loop of new face not complete" << std::endl;
			abort();
		}
	}
	if (k0 < new_face_halfedges.size()-1) {
		std::cerr << "loop of new face does not use all new halfedges" << std::endl;
		abort();
	}
	if (new_face_halfedges.back().first != F_outside.back() ||
		new_face_halfedges.back().second != F_outside.front()) {
		std::cerr << "loop of new face does is not closed by last new halfedge" << std::endl;
		abort();
	}
	// finally copy to inside shell in reverse order
	std::copy(F_outside.rbegin(), F_outside.rend(), std::back_inserter(F_inside));

	// in case original shell is kept, return pair of shell indices
	if (keep_original_shell)
		return std::pair<int, int>(si_inside, si_outside);
	// otherwise overwrite original shell with outside shell and remove outside shell again
	copy_shell(si_outside, si);
	del_shell(si_outside);
	return std::pair<int, int>(si_inside, si);
}

/// compute the intersection polygon between a shell and a plane
template <typename T, cgv::type::int32_type TCDim>
std::vector<typename convex_polyhedron<T, TCDim>::vertex_type> convex_polyhedron<T, TCDim>::compute_intersection_polygon(unsigned si, const plane_type& plane, T epsilon, T epsilon_flexibiliy) const
{
	// first compute vertex locations
	std::vector<VertexPlaneLocation> vertex_locations;
	std::vector<T> vertex_signed_distances;
	compute_vertex_locations(plane, vertex_locations, epsilon, &vertex_signed_distances);

	// next compute face locations and check for invalid cases
	std::vector<FacePlaneLocation> face_locations;
	if (!check_face_locations(si, vertex_locations, face_locations)) {
		std::cerr << "no valid face locations found" << std::endl;
		return std::vector<vertex_type>();
	}

	// check if intersection is empty
	if (shell_location_side(face_locations) != VPL_TOUCH)
		return std::vector<vertex_type>();

	// check if intersection corresponds to a touching face
	for (unsigned fi = 0; fi < face_locations.size(); ++fi) {
		if (face_locations[fi] == FPL_TOUCH_FACE) {
			std::vector<vertex_type> intersection;
			const face_type& F = face(fi, si);
			if (orientation_match(face_plane(fi, si), plane)) {
				for (auto v = F.begin(); v != F.end(); ++v)
					intersection.push_back(vertices[*v]);
			}
			else {
				for (auto v = F.rbegin(); v != F.rend(); ++v)
					intersection.push_back(vertices[*v]);
			}
			return intersection;
		}
	}
	// collect halfedges of face of new plane
	std::vector<std::pair<int, int> > new_face_halfedges;
	// use a map to hash the newly created vertices per edge
	std::map<std::pair<int, int>, int> edge_vertex_indices;
	// use a map to hash touched edges with their locations to allow adding edges of the new face
	std::map<std::pair<int, int>, FacePlaneLocation> edge_touch_events;
	// keep a vector of all vertices created on split edges
	std::vector<vertex_type> split_vertices;

	// go through all faces
	for (unsigned fi = 0; fi < get_nr_faces(si); ++fi) {
		switch (face_locations[fi]) {
		// outside faces can only contribute touching edges
		case FPL_OUTSIDE + FPL_TOUCH_EDGE:
		case FPL_INSIDE + FPL_TOUCH_EDGE:
		{
			// check touching edges for matching insides to also consider them for edges of the new face
			std::pair<int, int> halfedge = extract_touching_halfedge(face(fi, si), vertex_locations);
			std::pair<int, int> edge(std::min(halfedge.first, halfedge.second), std::max(halfedge.first, halfedge.second));
			auto ete_iter = edge_touch_events.find(edge);
			// if yes  
			if (ete_iter != edge_touch_events.end()) {
				// check for opposite side
				if (ete_iter->second != face_locations[fi]) {
					if (ete_iter->second == FPL_OUTSIDE + FPL_TOUCH_EDGE)
						std::swap(halfedge.first, halfedge.second);
					new_face_halfedges.push_back(halfedge);
				}
				// erase matched and unmatched events
				edge_touch_events.erase(ete_iter);
			}
			// otherwise create new edge touch event
			else
				edge_touch_events[edge] = face_locations[fi];
			break;
		}
		// in the split cases we need to iterate the halfedges
		case FPL_SPLIT_TWO_VERTICES:
		case FPL_SPLIT_VERTEX_EDGE:
		case FPL_SPLIT_TWO_EDGES:
		{
			std::vector<int> split_vis;
			const face_type& F = face(fi, si);
			int last_vi = F.back();
			VertexPlaneLocation last_vertex_location = vertex_locations[last_vi];
			for (int vi : F) {
				VertexPlaneLocation vertex_location = vertex_locations[vi];
				// touching vertices split the face
				if (vertex_location == VPL_TOUCH) {
					split_vis.push_back(vi);
					if (split_vis.size() == 2 && last_vertex_location == VPL_OUTSIDE)
						std::swap(split_vis[0], split_vis[1]);
				}
				// if previous halfedge is split, generate split vertex and add to both loops
				else {
					if (last_vertex_location != VPL_TOUCH && vertex_location != last_vertex_location) {
						// check if split vertex has already been created on inverse halfedge
						std::pair<int, int> edge(std::min(last_vi, vi), std::max(last_vi, vi));
						auto evi_iter = edge_vertex_indices.find(edge);
						int split_vi;
						// if yes, get vertex index and remove entry from map
						if (evi_iter != edge_vertex_indices.end()) {
							split_vi = evi_iter->second;
							edge_vertex_indices.erase(evi_iter);
						}
						// otherwise create new vertex on edge by affine interpolation according to unsigned distances
						else {
							split_vi = -1-(int)split_vertices.size();
							T distance = abs(vertex_signed_distances[vi]);
							T last_distance = abs(vertex_signed_distances[last_vi]);
							T lambda = last_distance / (last_distance + distance);
							vertex_type split_vertex;
							split_vertex.position = (T(1) - lambda) * position(last_vi) + lambda * position(vi);
							split_vertex.texcoord = (T(1) - lambda) * texcoord(last_vi) + lambda * texcoord(vi);
							split_vertices.push_back(split_vertex);
							edge_vertex_indices[edge] = split_vi;
						}
						split_vis.push_back(split_vi);
						if (split_vis.size() == 2 && last_vertex_location == VPL_OUTSIDE)
							std::swap(split_vis[0], split_vis[1]);
					}
				}
				last_vi = vi;
				last_vertex_location = vertex_location;
			}
			if (split_vis.size() != 2) {
				std::cerr << "expected two split vertices but found " << split_vis.size() << std::endl;
				abort();
			}
			new_face_halfedges.push_back(std::pair<int, int>(split_vis[0], split_vis[1]));
			break;
		}
		case FPL_INSIDE + FPL_TOUCH_NOTHING:
		case FPL_INSIDE + FPL_TOUCH_VERTEX:
		case FPL_OUTSIDE + FPL_TOUCH_NOTHING:
		case FPL_OUTSIDE + FPL_TOUCH_VERTEX:
			break;
		default:
			std::cerr << "did not expect face plane location " << face_locations[fi] << std::endl;
			abort();
		}
	}
	// finally we can construct intersection polygon
	std::vector<vertex_type> intersection;

	// first link the new face for the outside shell
	intersection.resize(new_face_halfedges.size());
	int fst_vi  = new_face_halfedges[0].first;
	int last_vi = new_face_halfedges[0].second;
	intersection[0] = fst_vi < 0 ? split_vertices[-fst_vi-1] : vertices[fst_vi];
	intersection[1] = last_vi < 0 ? split_vertices[-last_vi-1] : vertices[last_vi];
	unsigned k0 = 1;
	for (unsigned i = 2; i < intersection.size(); ++i) {
		bool found = false;
		for (unsigned k = k0; k < new_face_halfedges.size(); ++k) {
			if (new_face_halfedges[k].first == last_vi) {
				last_vi = new_face_halfedges[k].second;
				intersection[i] = last_vi < 0 ? split_vertices[-last_vi-1] : vertices[last_vi];
				if (k > k0)
					std::swap(new_face_halfedges[k0], new_face_halfedges[k]);
				++k0;
				found = true;
				break;
			}
		}
		if (!found) {
			std::cerr << "loop of new face not complete" << std::endl;
			abort();
		}
	}
	if (k0 < new_face_halfedges.size() - 1) {
		std::cerr << "loop of new face does not use all new halfedges" << std::endl;
		abort();
	}
	if (new_face_halfedges.back().first != last_vi ||
		new_face_halfedges.back().second != fst_vi) {
		std::cerr << "loop of new face does is not closed by last new halfedge" << std::endl;
		abort();
	}
	return intersection;
}
		}
	}
}

#include <cgv/config/lib_end.h>