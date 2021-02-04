#include "convex_polyhedron.h"

namespace cgv {
	namespace media {
		namespace mesh {


int convex_polyhedron_base::box_get_face_vertex_index(int fi, int ci) 
{
	/*
	67
	45
	--
	23
	01
	*/
	static unsigned face_vertex_indices[] = {
		1, 3, 7, 5,
		0, 4, 6, 2,

		2, 6, 7, 3,
		1, 5, 4, 0,

		4, 5, 7, 6,
		0, 2, 3, 1
	};
	return face_vertex_indices[4 * fi + ci];
}

void convex_polyhedron_base::change_orientation(face_type& F)
{
	for (unsigned i = 0; i < F.size() / 2; ++i)
		std::swap(F[i], F[F.size() - 1 - i]);
}

/// check whether all vertices lay on one side of the plane or on it, but not on the other; return side or VPL_TOUCH if vertices lay on both sides
convex_polyhedron_base::VertexPlaneLocation convex_polyhedron_base::vertex_location_side(const std::vector<VertexPlaneLocation>& vertex_locations)
{
	unsigned location_counts[3] = { 0, 0, 0 };
	for (auto vertex_location : vertex_locations)
		++location_counts[to_index(vertex_location)];

	if (location_counts[0] == 0)
		return VPL_OUTSIDE;
	if (location_counts[2] == 0)
		return VPL_INSIDE;
	return VPL_TOUCH;
}

/// check whether all faces of a shell lay on one side of the plane or touch it, but not on the other side; return side or VPL_TOUCH if shell is intersected by plane
convex_polyhedron_base::VertexPlaneLocation convex_polyhedron_base::shell_location_side(const std::vector<FacePlaneLocation>& face_locations)
{
	// first check for cases where the complete shell is on one side only
	unsigned face_nr_outside = 0;
	unsigned face_nr_inside = 0;
	unsigned face_nr_split = 0;
	for (FacePlaneLocation face_location : face_locations) {
		switch (face_location) {
		case FPL_OUTSIDE + FPL_TOUCH_NOTHING:
		case FPL_OUTSIDE + FPL_TOUCH_VERTEX:
		case FPL_OUTSIDE + FPL_TOUCH_EDGE:
			++face_nr_outside;
			break;
		case FPL_INSIDE + FPL_TOUCH_NOTHING:
		case FPL_INSIDE + FPL_TOUCH_VERTEX:
		case FPL_INSIDE + FPL_TOUCH_EDGE:
			++face_nr_inside;
			break;
		case FPL_TOUCH_FACE:
			break;
		case FPL_SPLIT_TWO_VERTICES:
		case FPL_SPLIT_VERTEX_EDGE:
		case FPL_SPLIT_TWO_EDGES:
			++face_nr_split;
			break;
		default:
			std::cerr << "found an invalid face location" << std::endl;
			abort();
			break;
		}
	}
	// check if no split is necessary
	if (face_nr_split > 0)
		return VPL_TOUCH;
	if (face_nr_inside == 0)
		return VPL_OUTSIDE;
	if (face_nr_outside == 0)
		return VPL_INSIDE;
	return VPL_TOUCH;
}


/// find a halfedge that touches the current clip plane
std::pair<int, int> convex_polyhedron_base::extract_touching_halfedge(const face_type& F, const std::vector<VertexPlaneLocation>& vertex_locations)
{
	std::pair<int, int> halfedge(F.back(), F.front());
	VertexPlaneLocation last_vertex_location = vertex_locations[halfedge.first];
	for (auto vi : F) {
		halfedge.second = vi;
		VertexPlaneLocation vertex_location = vertex_locations[vi];
		if (last_vertex_location == VPL_TOUCH && vertex_location == VPL_TOUCH)
			return halfedge;
		halfedge.first = vi;
		last_vertex_location = vertex_location;
	}
	std::cerr << "could not find touching halfedge in face" << std::endl;
	abort();
}

///
convex_polyhedron_base::FacePlaneLocation convex_polyhedron_base::get_face_plane_location(const face_type& F, const std::vector<VertexPlaneLocation>& vertex_locations) const
{
	// prepare counts
	unsigned vertex_nr_locations[3] = { 0, 0, 0 };
	unsigned edge_nr_touches = 0;
	unsigned edge_nr_splits = 0;
	unsigned nr_side_transitions = 0;

	// prepare loop over face vertex indices
	unsigned i = (unsigned)F.size() - 1;
	VertexPlaneLocation last_vertex_location = vertex_locations[F[i]];

	// determine side (vertex location excluding touch) at end of face vertices
	VertexPlaneLocation last_vertex_side = last_vertex_location;
	while (i > 0 && last_vertex_side == VPL_TOUCH)
		last_vertex_side = vertex_locations[F[--i]];
	// if all vertices touch, return face_touch-event
	if (last_vertex_side == VPL_TOUCH)
		return FPL_TOUCH_FACE;

	// loop face vertex indices and update counts
	for (i = 0; i < F.size(); ++i) {
		int vi = F[i];
		VertexPlaneLocation vertex_location = vertex_locations[vi];

		++vertex_nr_locations[to_index(vertex_location)];

		if (vertex_location == VPL_TOUCH) {
			if (last_vertex_location == VPL_TOUCH)
				++edge_nr_touches;
		}
		else {
			if (last_vertex_location != VPL_TOUCH && vertex_location != last_vertex_location)
				++edge_nr_splits;
			if (vertex_location != last_vertex_side)
				++nr_side_transitions;
			last_vertex_side = vertex_location;
		}
		last_vertex_location = vertex_location;
	}

	// analyze counts to locate face		
	FacePlaneLocation face_location = FPL_UNCONSIDERED_CASE;

	// handle side and touch cases
	if (vertex_nr_locations[to_index(VPL_INSIDE)] == 0 || vertex_nr_locations[to_index(VPL_OUTSIDE)] == 0) {
		face_location = vertex_nr_locations[to_index(VPL_INSIDE)] > 0 ? FPL_INSIDE : FPL_OUTSIDE;
		if (vertex_nr_locations[to_index(VPL_TOUCH)] == 1)
			(int&)face_location += (int)FPL_TOUCH_VERTEX;
		else if (vertex_nr_locations[to_index(VPL_TOUCH)] == 2) {
			if (edge_nr_touches == 1)
				(int&)face_location += (int)FPL_TOUCH_EDGE;
			else {
				if (edge_nr_touches != 0) {
					std::cerr << "unconsidered case (v#+|-=0, v#0=2, e#0>1)" << std::endl;
					abort();
				}
				face_location = FPL_TWO_NON_ADJACENT_VERTICES_TOUCH;
			}
		}
		else if(vertex_nr_locations[to_index(VPL_TOUCH)] != 0)
			face_location = FPL_UNCONSIDERED_CASE;
	}

	// handle valid and invalid single split cases
	if (nr_side_transitions == 2) {
		if (vertex_nr_locations[to_index(VPL_TOUCH)] == 2 && edge_nr_splits == 0)
			face_location = FPL_SPLIT_TWO_VERTICES;
		else if (vertex_nr_locations[to_index(VPL_TOUCH)] == 1 && edge_nr_splits == 1)
			face_location = FPL_SPLIT_VERTEX_EDGE;
		else if (vertex_nr_locations[to_index(VPL_TOUCH)] == 0 && edge_nr_splits == 2)
			face_location = FPL_SPLIT_TWO_EDGES;
		else if (vertex_nr_locations[to_index(VPL_TOUCH)] + edge_nr_splits > 2)
			face_location = FPL_TOUCH_AND_SPLIT;
		else
			face_location = FPL_UNCONSIDERED_CASE;
	}

	// handle remaining failure cases
	if (face_location == FPL_UNCONSIDERED_CASE) {
		if (vertex_nr_locations[to_index(VPL_TOUCH)] > 2)
			face_location = FPL_INCOMLETE_TOUCH_FACE;
		else if (nr_side_transitions % 2 == 1)
			face_location = FPL_ODD_TRANSITION_NUMBER;
		else if (nr_side_transitions > 2)
			face_location = FPL_MULTI_SPLIT;
		else
			face_location = FPL_UNCONSIDERED_CASE;
	}

	return face_location;
}


/// remove redundant vertices from mesh
void convex_polyhedron_base::remove_redundant_vertices()
{
	// compute vertex indices used in faces of shells
	std::vector<bool> vertex_index_used(get_nr_vertices(), false);
	for (const auto& S : shells)
		for (const auto& F : S)
			for (int vi : F)
				vertex_index_used[vi] = true;

	// compute new vertex indices
	std::vector<int> new_vertex_indices;
	new_vertex_indices.resize(get_nr_vertices());
	int new_vertex_index = 0;
	unsigned vi;
	for (vi = 0; vi < get_nr_vertices(); ++vi) {
		new_vertex_indices[vi] = new_vertex_index;
		if (vertex_index_used[vi])
			++new_vertex_index;
	}

	// remove unused vertices
	for (vi = (unsigned)get_nr_vertices(); vi > 0;)
		if (!vertex_index_used[--vi])
			del_vertex(vi);

	// update vertex indices in shell faces
	for (auto& S : shells)
		for (auto& F : S)
			for (int& vi : F)
				vi = new_vertex_indices[vi];
}

/*
void test_convex_polyhedron()
{
	convex_polyhedron<float,3> sdmt;
	sdmt.construct_box(convex_polyhedron<float>::box_type(convex_polyhedron<float>::point_type(-1, -1, -1), convex_polyhedron<float>::point_type(1, 1, 1)));
	sdmt.split_at_plane(0, convex_polyhedron<float>::plane_type(1, 0, 0, 0));
	sdmt.clip_to_outside_of_plane(0, convex_polyhedron<double>::plane_type(1, 0, 0, 0));

	convex_polyhedron<double,3> sdmT;
	sdmT.construct_box(convex_polyhedron<double>::box_type(convex_polyhedron<double>::point_type(-1, -1, -1), convex_polyhedron<double>::point_type(1, 1, 1)));
	sdmT.split_at_plane(0, convex_polyhedron<double>::plane_type(1, 0, 0, 0));
	sdmT.clip_to_inside_of_plane(0, convex_polyhedron<double>::plane_type(1, 0, 0, 0));
}
*/
		}
	}
}
