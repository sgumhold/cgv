#pragma once

#include "corner_connectivity.h"

#include "lib_begin.h"

/// simple triangle mesh data structure using the corner data structure to store neighbor relations
class CGV_API ext_corner_connectivity : public corner_connectivity
{
protected:
	/// list of corner indices for each vertex
	mutable std::vector<unsigned int> V;
public:
	/**@name construction*/
	//@{
	/// construct empty triangle mesh
	ext_corner_connectivity();
	/// remove all triangles
	void clear_triangles();
	/// add a triangle with all edges boundary edges
	void add_triangle(unsigned int v0, unsigned int v1, unsigned int v2);
	/// add a triangle with the given cornern information
	void add_triangle(const corner& c0, const corner& c1, const corner& c2);
	//@}

	/**@name access and navigation */
	//@{
	/// check if the ci_of_vi field is consistent up to vertex n-1
	void is_ci_of_vi_consistent(unsigned int n) const;
	/// return a reference to the corner index of a vertex
	unsigned int& ci_of_vi(unsigned int vi);
	/// return the corner index of a vertex
	unsigned int ci_of_vi(unsigned int vi) const;
	//@}

	/**@name modification*/
	//@{
	/// flip an edge that is opposite to a corner c0
	void flip_edge(unsigned int c0);
	/// split a triangle given by a corner into 3 triangles at the given vertex index
	void split_triangle_at_vertex(unsigned int c0, unsigned int vi);
	/// build a triangle with vertex vi on the border edge specified by the opposite corner c0
	void build_triangle_on_border_edge(unsigned int c0, unsigned int vi);
	//@}
};


/// return a reference to the vertex index of a corner
inline unsigned int& ext_corner_connectivity::ci_of_vi(unsigned int vi)
{
	while (vi >= V.size())
		V.push_back(-1);
	return V[vi];
}
/// return the vertex index of a corner
inline unsigned int ext_corner_connectivity::ci_of_vi(unsigned int vi) const
{
	while (vi >= V.size())
		V.push_back(-1);
	return V[vi];
}

#include <cgv/config/lib_end.h>