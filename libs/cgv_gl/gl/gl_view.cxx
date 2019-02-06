#include <cgv/base/base.h>
#include "gl_view.h"
#include <cgv/render/context.h>

namespace cgv {
	namespace render {
		namespace gl {

gl_view::gl_view()
{
	z_near = 0.0001;
	z_far = 100;
}

double gl_view::get_z_near() const 
{
	return z_near; 
}

void gl_view::set_z_near(double z) 
{
	z_near = z; 
}

double gl_view::get_z_far() const 
{
	return z_far; 
}

void gl_view::set_z_far(double z) 
{
	z_far = z; 
}

///
void gl_view::set_scene_extent(const dbox3& _box)
{
	scene_extent = _box;
}

///
gl_view::dbox3 gl_view::get_scene_extent() const
{
	if (scene_extent.is_valid())
		return scene_extent;
	return dbox3(dbox3::pnt_type(-1, -1, -1), dbox3::pnt_type(1, 1, 1));
}

void gl_view::set_default_view()
{
	set_view_dir(0, 0, -1);
	set_view_up_dir(0, 1, 0);
	set_focus(get_scene_extent().get_center());
	set_y_extent_at_focus(1.5*get_scene_extent().get_extent()(1));
}

/// transform a z value in eye-coordinates (should be negative!) to device coordinate
double gl_view::get_z_D(double z_eye, double z_near, double z_far)
{
	double C = -(z_far + z_near) / (z_far - z_near);
	double D = -2.0*z_far*z_near / (z_far - z_near);
	return 0.5 - 0.5*(D + C*z_eye) / z_eye;
}

void gl_view::compute_clipping_planes(double& z_near_derived, double& z_far_derived, bool clip_relative_to_extent) const
{
	compute_clipping_planes(*this, z_near_derived, z_far_derived, clip_relative_to_extent);
}

/// compute clipping planes according to given view adapted to the current scene extent, z_near_derived is at least z_near and as large as possible to include the scene, similarly z_far_derived is as small as possible  
void gl_view::compute_clipping_planes(const cgv::render::view& view, double& z_near_derived, double& z_far_derived, bool clip_relative_to_extent) const
{
	// compute eye and focus point
	dvec3 foc = view.get_focus();
	dvec3 eye = view.get_eye();
	dvec3 view_dir = view.get_view_dir();
	double z_eye = dot(eye, view_dir);

	// compute the clipping planes based on the eye and scene extent
	z_near_derived = z_near;
	z_far_derived = z_far;
	if (scene_extent.is_valid()) {
		dbox3 B = scene_extent;
		B.scale(1.25);
		double z_min = dot(B.get_corner(0), view_dir);
		double z_max = z_min;
		for (unsigned int i = 1; i<8; ++i) {
			double new_z = dot(B.get_corner(i), view_dir);
			if (new_z< z_min)
				z_min = new_z;
			if (new_z > z_max)
				z_max = new_z;
		}
		z_min -= z_eye;
		z_max -= z_eye;
		if (z_min > z_near)
			z_near_derived = z_min;
		if (z_max > z_near)
			z_far_derived = z_max;
	}
	else if (clip_relative_to_extent) {
		z_near_derived = y_extent_at_focus*z_near;
		z_far_derived = y_extent_at_focus*z_far;
	}
}



		}
	}
}