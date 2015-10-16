#pragma once

#include "point_2d.h"
#include <vector>

#include "lib_begin.h"

/// simple triangle mesh data structure using the corner data structure to store neighbor relations
template <typename ta_coord_type, class ta_point_type = point_2d<ta_coord_type> >
class CGV_API mesh_geometry
{
public:
	/// declare coordinate type
	typedef ta_coord_type coord_type;
	/// declare point type
	typedef ta_point_type point_type;
	/// different sampling types
	enum SamplingType { ST_REGULAR, ST_RANDOM, ST_STRATIFIED };
	/// different sampling distributions
	enum DistributionType { DT_UNIFORM, DT_NORMAL };
	/// different sampling types for sample data set generation
	enum GeneratorType { GT_RANDOM, GT_PSEUDO_RANDOM_DEFAULT, GT_PSEUDO_RANDOM_MT };
	/// different shape types for sample data set generation
	enum ShapeType { ST_SQUARE, ST_TRIANGLE, ST_CIRCLE, ST_SPIRAL };
	/// different sampling strategies
	enum SamplingStrategy { SS_REJECTION, SS_TRANSFORM };
protected:
	/// list of points
	std::vector<point_type> P;
	/// transform a random point to a given shape
	static point_type transform(const point_type& p, ShapeType shape);
public:
	/**@name construction*/
	//@{
	/// construct empty triangle mesh
	mesh_geometry();
	/// generate random points
	void generate_sample_data_set(unsigned int n, SamplingType sampling, DistributionType dt, GeneratorType gt, ShapeType shape, SamplingStrategy ss, bool do_random_shuffle);
	/// remove all vertices 
	void clear_geometry();
	/// return the number of vertices
	unsigned int get_nr_vertices() const;
	/// return a reference to the vertex location 
	point_type& p_of_vi(unsigned int vi);
	/// return the location of a vertex
	const point_type& p_of_vi(unsigned int vi) const;
	/// add a new point to the geometry
	void add_point(const point_type& p);
	/// check if point p is outside with respect to the line through p1 and p2
	static bool is_outside(const point_type& p, const point_type& p1, const point_type& p2);
	/// check if point p is inside with respect to the line through p1 and p2
	static bool is_inside(const point_type& p, const point_type& p1, const point_type& p2);
	/// check if point p is incident to the segment from p1 to p2
	static bool is_incident(const point_type& p, const point_type& p1, const point_type& p2);
	/// check if point p projects inside the segment from p1 to p2
	static bool is_between(const point_type& p, const point_type& p1, const point_type& p2);
	/// compute the squared distance between two points
	static coord_type sqr_dist(const point_type& p0, const point_type& p1);
	/// compute the circum center of a triangle through p0, p1 and p2
	static point_type compute_circum_center(const point_type& p0, const point_type& p1, const point_type& p2);
};

#ifdef CGV_IS_STATIC

/// return the number of vertices
template <typename C, class P>
inline unsigned int mesh_geometry<C,P>::get_nr_vertices() const
{
	return (unsigned int) P.size();
}
/// return a reference to the vertex location 
template <typename C, class P>
inline typename mesh_geometry<C,P>::point_type& mesh_geometry<C,P>::p_of_vi(unsigned int vi)
{
	return P[vi];
}
/// return the location of a vertex
template <typename C, class P>
inline const typename mesh_geometry<C,P>::point_type& mesh_geometry<C,P>::p_of_vi(unsigned int vi) const
{
	return P[vi];
}

#endif

#include <cgv/config/lib_end.h>