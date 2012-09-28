#pragma once

#include "point_cloud.h"
#include "neighbor_graph.h"

#include "lib_begin.h"

class CGV_API normal_estimator
{
public:
	typedef point_cloud::coord_type coord_type;
	typedef point_cloud::Pnt Pnt;
	typedef point_cloud::Nml Nml;
	typedef point_cloud::Nml Vec;
	typedef point_cloud::Box Box;

	point_cloud& pc;
	neighbor_graph& ng;

	coord_type normal_quality_exp;
	coord_type smoothing_scale;
	coord_type noise_to_sampling_ratio;
	bool use_orientation;

	coord_type compute_normal_quality(const Nml& n1, const Nml& n2) const;
public:
	/// cosntruct from point cloud and neighbor graph
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
