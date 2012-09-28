#pragma once

#include "mesh_geometry_reference.h"
#include "delaunay_mesh.h"

#include "lib_begin.h"

/// simple triangle mesh data structure using the corner data structure to store neighbor relations
template <class ta_delaunay_mesh = delaunay_mesh<> >
class CGV_API delaunay_mesh_with_hierarchy : public ta_delaunay_mesh
{
public:
	/// type of triangle mesh
	typedef ta_delaunay_mesh delaunay_mesh_type;
	///
	typedef typename delaunay_mesh_type::corner corner;
	///
	typedef typename delaunay_mesh_type::point_type point_type;
	///
	typedef typename delaunay_mesh_type::coord_type coord_type;
	///
	typedef typename delaunay_mesh_type::point_location_info point_location_info;
	///
	typedef typename delaunay_mesh_type::vertex_insertion_info vertex_insertion_info;
	///
	typedef delaunay_mesh<triangle_mesh<mesh_geometry_reference<coord_type,point_type> > > hierarchy_level_type;
protected:
	/// hierarchy of delaunay triangulations used to speed up nearest neighbor search
	std::vector<hierarchy_level_type*> H;
	///
	unsigned int hierarchy_factor;
	///
	unsigned int next_hierarchy;
public:
	/**@name construction*/
	//@{
	/// construct empty delaunay mesh
	delaunay_mesh_with_hierarchy();
	/// implement copy constructor to set reference geometries in hierarchy correctly
	delaunay_mesh_with_hierarchy(const delaunay_mesh_with_hierarchy& dm);
	///
	void set_hierarchy_factor(unsigned int hf);
	/// configure the hierarchy settings for the given number of points or if not specified for the current number of points
	void configure_hierarchy(unsigned int n = -1);
	/// reimplement clear to remove all hierarchy levels
	void clear();
	//@}

	/**@name geometric predicates*/
	//@{
	/// overload point localization to use hierarchy, the ci_start argument is ignored as its existence on the coarsest hierarchy level cannot be guaranteed
	point_location_info localize_point(const point_type& p, unsigned int ci_start = 0) const;
	/// reimplement nearest neighbor search using the hierarchy
	unsigned int find_nearest_neighbor(const point_type& p, unsigned int ci_start = 0) const;
	//@}
	/// reimplement to construct the hierarchy levels
	vertex_insertion_info insert_vertex(unsigned int vi, unsigned int ci_start = 0, std::vector<unsigned int>* touched_corners = 0);
};

#include <cgv/config/lib_end.h>