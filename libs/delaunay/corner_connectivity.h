#pragma once

#include <vector>

#include "lib_begin.h"

/// simple triangle mesh data structure using the corner data structure to store neighbor relations
class CGV_API corner_connectivity
{
public:
	/// the opposite info stores whether the corner is opposite to a border edge and if not the index of the opposite corner
	struct opposite_info {
		/// whether the opposite edge is a border edge
		bool is_opposite_to_border : 1;
		/// index of opposite corner
		unsigned int ci : 31;
		/// construct as opposite to border
		opposite_info() : ci(0), is_opposite_to_border(true) {}
		/// construct from index of opposite corner
		opposite_info(unsigned int _ci) : ci(_ci), is_opposite_to_border(false) {}
	};
	/// type of triangle corner includes vertex index and index of opposite corner
	struct corner {
		/// index of corner vertex
		unsigned int vi;
		/// store the opposite information
		opposite_info opposite;
		/// construct from vertex index opposite to corner
		corner(unsigned int _vi = 0) : vi(_vi) {}
		/// construct from vertex index and opposite corner
		corner(unsigned int _vi, unsigned int _ci) : vi(_vi), opposite(_ci) {}
		/// construct from vertex index and opposite corner
		corner(unsigned int _vi, opposite_info o) : vi(_vi), opposite(o) {}
	};
	/// cyclic iterator around a vertex taking care of border case
	struct neighbor_cycler 
	{
	protected:
		bool next_is_boundary;
		unsigned int ci;
		unsigned int cj;
		const corner_connectivity* C;
	public:
		/// construct neighbor cycler from connectivity and index of corner incident to the to be cycled vertex
		neighbor_cycler(const corner_connectivity* _C, unsigned int _ci) :
			C(_C), ci(_ci), cj(-1), next_is_boundary(false) {}
		/// check if one cycle is complete
		bool cycle_complete() const { return ci == cj; }
		/// query the corner index of the current neighbor
		unsigned int ci_of_nbr() const { 
			unsigned int ck = ( (cj == -1) ? ci : cj);
			return C->next(ck); 
		}
		/// query the corner index of the edge from vertex to neighbor
		unsigned int ci_of_edge() const { 
			unsigned int ck = ci_of_nbr();
			if (next_is_boundary)
				return C->prev(ck);
			else
				return C->next(ck); 
		}
		/// cycle to next neighbor
		void next() { 
			if (next_is_boundary) {
				cj = C->next(C->prev_on_border(cj));
				next_is_boundary = false;
			}
			else {
				cj = ci_of_nbr();
				if (C->is_opposite_to_border(cj))
					next_is_boundary = true;
				else
					cj = C->next(C->inv(cj));
			}
		}
	};
	/// cyclic iterator around a vertex taking care of border case
	struct corner_cycler 
	{
	protected:
		unsigned int c0;
		unsigned int cj;
		const corner_connectivity* C;
	public:
		/// construct neighbor cycler from connectivity and index of corner incident to the to be cycled vertex
		corner_cycler(const corner_connectivity* _C, unsigned int _ci) :
			C(_C), c0(_ci), cj(-1) {}
		/// check if one cycle is complete
		bool cycle_complete() const { return c0 == cj; }
		/// query the corner index
		unsigned int ci() const { return cj == -1 ? c0 : cj; }
		/// cycle to next neighbor
		void next() { 
			cj = C->next(ci());
			if (C->is_opposite_to_border(cj))
				cj = C->next(C->prev_on_border(cj));
			else
				cj = C->next(C->inv(cj));
		}
	};
protected:
	/// list of corners (3 times more entries than triangles)
	std::vector<corner> C;
	/// return a reference to the opposite info of a corner
	opposite_info& opposite(unsigned int ci);
	/// return the opposite info of a corner
	const opposite_info& opposite(unsigned int ci) const;
public:
	/**@name construction*/
	//@{
	/// construct empty triangle mesh
	corner_connectivity();
	/// remove all triangles
	void clear_triangles();
	/// add a triangle with all edges boundary edges
	void add_triangle(unsigned int v0, unsigned int v1, unsigned int v2);
	/// add a triangle with the given cornern information
	void add_triangle(const corner& c0, const corner& c1, const corner& c2);
	//@}

	/**@name access and navigation */
	//@{
	/// return the number of triangles
	unsigned int get_nr_triangles() const;
	/// return the number of corners
	unsigned int get_nr_corners() const;
	/// return a neighbor cycler
	neighbor_cycler get_nbr_cycler(unsigned int ci) const;
	/// return a corner cycler
	corner_cycler get_corner_cycler(unsigned int ci) const;
	/// return a reference to the vertex index of a corner
	unsigned int& vi_of_ci(unsigned int ci);
	/// return the vertex index of a corner
	unsigned int vi_of_ci(unsigned int ci) const;
	/// return the triangle index of a corner
	static unsigned int ti_of_ci(unsigned int ci);
	/// return the index of the first corner of a triangle
	static unsigned int ci_of_ti(unsigned int ti);
	/// return the next corner in the same triangle
	unsigned int next(unsigned int ci) const;
	/// return the previous corner in the same triangle
	unsigned int prev(unsigned int ci) const;
	/// return the inverse corner in the opposite edge adjacent triangle
	inline unsigned int inv(unsigned int ci) const;
	/// check if the opposite edge is a border edge
	bool is_opposite_to_border(unsigned int ci) const;
	/// find the corner that is opposite to the border edge which follows the border edge opposite to the given corner index
	unsigned int next_on_border(unsigned int ci) const;
	/// find the corner that is opposite to the border edge which preceeds the border edge opposite to the given corner index
	unsigned int prev_on_border(unsigned int ci) const;
	//@}

	/**@name modification*/
	//@{
	/// check if an edge is flipable
	bool is_flipable(unsigned int c0) const;
	/// flip an edge that is opposite to a corner c0
	void flip_edge(unsigned int c0);
	/// split a triangle given by a corner into 3 triangles at the given vertex index
	void split_triangle_at_vertex(unsigned int c0, unsigned int vi);
	/// build a triangle with vertex vi on the border edge specified by the opposite corner c0
	void build_triangle_on_border_edge(unsigned int c0, unsigned int vi);
	/// build a triangle in between the border edge opposite to the given corner and the next edge along the border
	void build_triangle_connection_to_next_border_edge(unsigned int c0);
	/// build a triangle in between the border edge opposite to the given corner and the previous edge along the border
	void build_triangle_connection_to_prev_border_edge(unsigned int c0);
	//@}
};


/// return a neighbor iterator
inline corner_connectivity::neighbor_cycler corner_connectivity::get_nbr_cycler(unsigned int ci) const
{
	return neighbor_cycler(this,ci);
}
/// return a neighbor iterator
inline corner_connectivity::corner_cycler corner_connectivity::get_corner_cycler(unsigned int ci) const
{
	return corner_cycler(this,ci);
}
/// return a reference to the opposite info of a corner
inline corner_connectivity::opposite_info& corner_connectivity::opposite(unsigned int ci)
{
	return C[ci].opposite;
}
/// return the opposite info of a corner
inline const corner_connectivity::opposite_info& corner_connectivity::opposite(unsigned int ci) const
{
	return C[ci].opposite;
}
/// return the number of triangles
inline unsigned int corner_connectivity::get_nr_triangles() const
{
	return get_nr_corners()/3;
}
/// return the number of corners
inline unsigned int corner_connectivity::get_nr_corners() const
{
	return (unsigned int) C.size();
}
/// return a reference to the vertex index of a corner
inline unsigned int& corner_connectivity::vi_of_ci(unsigned int ci)
{
	return C[ci].vi;
}
/// return the vertex index of a corner
inline unsigned int corner_connectivity::vi_of_ci(unsigned int ci) const
{
	return C[ci].vi;
}
/// return the triangle index of a corner
inline unsigned int corner_connectivity::ti_of_ci(unsigned int ci)
{
	return ci/3;
}
/// return the index of the first corner of a triangle
inline unsigned int corner_connectivity::ci_of_ti(unsigned int ti)
{
	return ti*3;
}
/// return the next corner in the same triangle
inline unsigned int corner_connectivity::next(unsigned int ci) const 
{
	return ci_of_ti(ti_of_ci(ci))+(ci+1)%3;
}
/// return the previous corner in the same triangle
inline unsigned int corner_connectivity::prev(unsigned int ci) const 
{
	return ci_of_ti(ti_of_ci(ci))+(ci+2)%3;
}
/// return the inverse corner in the opposite edge adjacent triangle
inline unsigned int corner_connectivity::inv(unsigned int ci) const
{
	return opposite(ci).ci;
}
/// check if the opposite edge is a border edge
inline bool corner_connectivity::is_opposite_to_border(unsigned int ci) const
{
	return opposite(ci).is_opposite_to_border;
}

#include <cgv/config/lib_end.h>