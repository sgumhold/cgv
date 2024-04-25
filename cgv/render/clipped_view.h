#pragma once

#include <cgv/render/view.h>
#include <cgv/media/axis_aligned_box.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {

/** extends the view class with information on z clipping planes and scene extent
    and supports clipping plane computation from scene extent. */
class CGV_API clipped_view : public view
{
protected:
	double z_near;
	double z_far;
	dbox3 scene_extent;
public:
	/// construct a parallel view with focus in the world origin looking in negative z-direction and the y-direction as up-direction with an extent of +-1
	clipped_view();
	/// compute clipping planes adapted to the current scene extent, z_near_derived is at least z_near and as large as possible to include the scene, similarly z_far_derived is as small as possible  
	void compute_clipping_planes(double& z_near_derived, double& z_far_derived, bool clip_relative_to_extent = false) const;
	/// compute clipping planes according to given view adapted to the current scene extent, z_near_derived is at least z_near and as large as possible to include the scene, similarly z_far_derived is as small as possible  
	void compute_clipping_planes(const view& view, double& z_near_derived, double& z_far_derived, bool clip_relative_to_extent = false) const;
	/// compute clipping planes according to given view adapted to the current scene extent, z_near_derived is at least z_near and as large as possible to include the scene, similarly z_far_derived is as small as possible  
	void compute_clipping_planes(const dvec3& eye, const dvec3& view_dir, double& z_near_derived, double& z_far_derived, bool clip_relative_to_extent = false) const;

	/**@name getter and setter methods*/
	//@{
	/// return the currently set z-value for the z-near clipping plane
	double get_z_near() const;
	/// reference to z_near value for ui construction
	double& ref_z_near() { return z_near; }
	///set the z-value for the z-near clipping plane
	virtual void set_z_near(double z);
	/// return the currently set z-value for the z-far clipping plane
	double get_z_far() const;
	/// reference to z_far value for ui construction
	double& ref_z_far() { return z_far; }
	/// set the z-value for the z-far clipping plane
	virtual void set_z_far(double z);
	/// transform a z value in eye-coordinates (should be negative!) to device coordinate
	static double get_z_D(double z_eye, double z_near, double z_far);
	/// set the extent of the scene in world coordinates used by the compute_clipping_planes functions to adapt the clipping planes to the scene
	virtual void set_scene_extent(const dbox3& _box);
	/// return the currently set scene extent
	dbox3 get_scene_extent() const;
	/// reset view with focus and y-extent based on current scene extent
	virtual void set_default_view();
	//@}
};

	}
}

#include <cgv/config/lib_end.h>