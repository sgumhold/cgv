#pragma once

#include "point_cloud.h"
#include "neighbor_graph.h"

#include "lib_begin.h"

/** the normal estimator class needs a reference to a point_cloud and a neighbor_graph and allows to
    compute [[bilaterally] weighted] least squares normals and to consistently orient the normals */
class CGV_API normal_estimator : public point_cloud_types
{
protected:
	point_cloud& pc;
	neighbor_graph& ng;
public:
	Crd normal_quality_exp;
	Crd smoothing_scale;
	Crd noise_to_sampling_ratio;
	bool use_orientation;

	Crd compute_normal_quality(const Nml& n1, const Nml& n2) const;
public:
	/// construct from point cloud and neighbor graph
	normal_estimator(point_cloud& _pc, neighbor_graph& _ng);
	/// smooth normals with bilateral weights
	void smooth_normals();
	/// (re)compute normals from neighbor graph and distance weights
	void compute_weighted_normals(bool reorient);
	/// recompute normals from neighbor graph and distance and normal weights
	void compute_bilateral_weighted_normals(bool reorient);
	/// recompute normals from neighbor graph and distance and normal weights
	void compute_plane_bilateral_weighted_normals(bool reorient);
	/// try to compute consistent normal orientation
	void orient_normals();
	/// orient normals towards given point
	void orient_normals(const Pnt& view_point);
};

#include <cgv/config/lib_end.h>
