#pragma once

#include <vector>
#include "point_cloud.h"

#include "lib_begin.h"

/** uses ann to provide a data structure to build a knn neighbor graph */
class CGV_API ann_tree : public point_cloud_types
{
protected:
	void* ann_impl;
	const point_cloud* pc;
	Cnt k;
public:
	/// construct
	ann_tree();
	/// destruct
	~ann_tree();
	/// clear the used memory
	void clear();
	/// check whether the tree has been built
	bool is_empty() const;
	/// build from complete point cloud
	void build(const point_cloud& pc);
	/// build from given components
	void build(const point_cloud& pc, const std::vector<Idx>& component_indices);
	/// provide necessary method for building a neighbor graph
	void extract_neighbors(Idx i, Idx k, std::vector<Idx>& N) const;
	/// addition query method to find the closest neighbor
	Idx find_closest(const Pnt& p) const;
	/// knn query that returns pointers to points
	void find_closest_points(const Pnt& p, Idx k, std::vector<const Pnt*>& knn) const;
};

#include <cgv/config/lib_end.h>