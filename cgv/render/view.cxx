#include <cgv/render/view.h>
#include <cgv/math/geom.h>

using namespace cgv::math;

namespace cgv {
	namespace render {

/// construct a parallel view with focus in the world origin looking in negative z-direction and the y-direction as up-direction with an extent of +-1
view::view() 
	: focus(0,0,0), view_up_dir(0,1,0), view_dir(0,0,-1), y_view_angle(0), y_extent_at_focus(2)
{
}

/// write access to focus point
view::dvec3& view::ref_focus() { return focus; }
/// write access to view up direction
view::dvec3& view::ref_view_up_dir() { return view_up_dir; }
/// write access to view dir
view::dvec3& view::ref_view_dir() { return view_dir; }
/// write access to view angle
double& view::ref_y_view_angle() { return y_view_angle; }
/// write access to extent at focus
double& view::ref_y_extent_at_focus() { return y_extent_at_focus; }

/// compute axis and angle of a rotation that the current view_dir and view_up_dir to the given target_view_dir and target_view_up_dir
int view::compute_axis_and_angle(const dvec3& target_view_dir, const dvec3& target_view_up_dir, 
	dvec3& axis, double& angle)
{
	dmat3 R = cgv::math::build_orthogonal_frame(view_dir, view_up_dir);
	R.transpose();
	R = cgv::math::build_orthogonal_frame(target_view_dir, target_view_up_dir)*R;
	dvec3 daxis;
	double dangle;
	int res = cgv::math::decompose_rotation_to_axis_and_angle(R, daxis, dangle);
	axis = daxis;
	angle = dangle;
	return res;
}

/// compute tan of half of y view angle, if ensure_non_zero is true, replace y view angles < 0.1 with 0.1
double view::get_tan_of_half_of_fovy(bool ensure_non_zero) const
{
	return tan(.008726646260*((ensure_non_zero && (y_view_angle <= 0.01)) ? 0.01 : y_view_angle));
}

///
const view::dvec3& view::get_focus() const
{
	return focus;
}
///
const view::dvec3& view::get_view_up_dir() const
{
	return view_up_dir;
}
///
const view::dvec3& view::get_view_dir() const
{
	return view_dir;
}
///
double view::get_y_view_angle() const
{
	return y_view_angle;
}
///
double view::get_y_extent_at_focus() const
{
	return y_extent_at_focus;
}
/// return the depth of the focus point
double view::get_depth_of_focus() const
{
	return 0.5f*y_extent_at_focus / get_tan_of_half_of_fovy(true);
}

///
double view::get_y_extent_at_depth(double depth, bool ensure_non_zero) const
{
	return 2.0f*depth*get_tan_of_half_of_fovy(ensure_non_zero);
}

/// 
void view::set_focus(const dvec3& foc) 
{
	focus = foc;
}
void view::set_focus(double x, double y, double z) { set_focus(dvec3(x,y,z)); }

///
void view::set_view_up_dir(const dvec3& vud)
{
	view_up_dir = vud;
}
void view::set_view_up_dir(double x, double y, double z) { set_view_up_dir(dvec3(x,y,z)); }
///
void view::set_view_dir(const dvec3& vd)
{
	view_dir = vd;
}
void view::set_view_dir(double x, double y, double z) { set_view_dir(dvec3(x,y,z)); }
///
void view::set_y_extent_at_focus(double ext)
{
	y_extent_at_focus = ext;
}
///
void view::set_y_view_angle(double angle)
{
	y_view_angle = angle;
}

/// return whether the y view angle is zero
bool view::is_parallel() const
{
	return y_view_angle == 0;
}

//! query the eye point, which is computed from focus, view dir, y extent at focus and y view angle
/*! With the y view angle approaching 0, the eye point moves infinitely far away. To avoid
numerical problems, the eye point is computed with an y view angle no less than 0.1.*/
const view::dvec3 view::get_eye() const
{
	return focus - (0.5f*y_extent_at_focus / get_tan_of_half_of_fovy(true))*view_dir;
}

//! set the view dir and y extent at focus keeping focus and y view angle such that get_eye() returns the passed point, return whether this was successful.
/*! Recomputes view up direction to make it orthogonal to view direction.
In the case that the eye point is identical to the current focus point the function fails and returns false.
If the current view angle is < 0.1, the view anlge 0.1 is used for eye point calculation */
bool view::set_eye_keep_view_angle(const dvec3& eye)
{
	dvec3 new_view_dir = focus - eye;
	dvec3::value_type l = new_view_dir.length();
	if (l < 10 * std::numeric_limits<dvec3::value_type>::epsilon())
		return false;
	dvec3::value_type inv_l = dvec3::value_type(1) / l;
	view_dir = inv_l*new_view_dir;
	y_extent_at_focus = get_y_extent_at_depth(l, true);
	return true;
}

//! set view dir and y view angle keeping focus and y extent such that get_eye() returns the passed point, return whether this was successful.
/*! Recomputes view up direction to make it orthogonal to view direction.
In the case that the eye point is identical to the current focus point the function fails and returns false.
*/
bool view::set_eye_keep_extent(const dvec3& eye)
{
	dvec3 new_view_dir = focus - eye;
	dvec3::value_type l = new_view_dir.length();
	if (l < 10 * std::numeric_limits<dvec3::value_type>::epsilon())
		return false;
	dvec3::value_type inv_l = dvec3::value_type(1) / l;
	view_dir = inv_l*new_view_dir;
	y_view_angle = atan(0.5*y_extent_at_focus*inv_l)*114.5915590;
	return true;
}

/// set the view according to the standard view lookat definition from eye, focus and view up direction.
void view::view_look_at_keep_extent(const dvec3& e, const dvec3& foc, const dvec3& vud)
{
	set_focus(foc);
	set_view_up_dir(vud);
	set_eye_keep_extent(e);
}

/// set the view according to the standard view lookat definition from eye, focus and view up direction.
void view::view_look_at_keep_view_angle(const dvec3& e, const dvec3& foc, const dvec3& vud)
{
	set_focus(foc);
	set_view_up_dir(vud);
	set_eye_keep_view_angle(e);
}

/// call this function before a drawing process to support viewport splitting inside the draw call via the activate/deactivate functions
void view::enable_viewport_splitting(unsigned nr_cols, unsigned nr_rows)
{}
/// check whether viewport splitting is activated and optionally set the number of columns and rows if corresponding pointers are passed
bool view::is_viewport_splitting_enabled(unsigned* nr_cols_ptr, unsigned* nr_rows_ptr) const
{
	return false;
}
/// disable viewport splitting
void view::disable_viewport_splitting()
{}
/// inside the drawing process activate the sub-viewport with the given column and row indices, always terminate an activated viewport with deactivate_split_viewport
void view::activate_split_viewport(context& ctx, unsigned col_index, unsigned row_index)
{}
/// deactivate the previously split viewport
void view::deactivate_split_viewport(context& ctx)
{}

/// make a viewport manage its own view
void view::enable_viewport_individual_view(unsigned col_index, unsigned row_index, bool enable)
{
}

/// check whether viewport manage its own view
bool view::does_viewport_use_individual_view(unsigned col_index, unsigned row_index) const
{
	return false;
}

/// access the view of a given viewport
view& view::ref_viewport_view(unsigned col_index, unsigned row_index)
{
	return *this;
}

void view::put_coordinate_system(dvec3& x, dvec3& y, dvec3& z) const
{
	z = -view_dir;
	z.normalize();
	x = cross(view_up_dir, z);
	x.normalize();
	y = cross(z, x);
}


void view::put_coordinate_system(vec3& x, vec3& y, vec3& z) const
{
	dvec3 dx, dy, dz;
	put_coordinate_system(dx, dy, dz);
	x = dx;
	y = dy;
	z = dz;
}

/// roll view around view direction by angle
void view::roll(double angle)
{
	view_up_dir = cgv::math::rotate(view_up_dir, view_dir, angle);
}

//! rotated view around axis by angle
/*! Axis is given by point and direction, where the point is in the image center and the given depth
and the axis points into a direction in image plane given through its screen x and screen y
coordinate. The length of the axis vector gives the rotation angle in radians.
Rotation around screen x direction corresponds to yaw and around screen y direction
to gear rotations. */
void view::rotate(double axis_direction_x, double axis_direction_y, double axis_point_depth)
{
	dvec3 x, y, z;
	put_coordinate_system(x, y, z);
	dvec3 axis_dir = axis_direction_x*x + axis_direction_y*y;
	double angle = axis_dir.length();
	if (angle < 10 * std::numeric_limits<double>::epsilon())
		return;
	axis_dir *= 1.0 / angle;
	dvec3 axis_point = get_eye() + axis_point_depth*view_dir;
	focus = cgv::math::rotate(focus - axis_point, axis_dir, angle) + axis_point;
	view_dir = cgv::math::rotate(view_dir, axis_dir, angle);
	view_up_dir = cgv::math::rotate(view_up_dir, axis_dir, angle);
}

/// move along view direction by given step length in world coordinates
void view::move(double step)
{
	focus += step*view_dir;
}

/// move in screen x and screen y directions by given step lengths in world coordinates
void view::pan(double step_x, double step_y)
{
	dvec3 x, y, z;
	put_coordinate_system(x, y, z);
	focus += step_x*x + step_y* y;
}

/// zoom by given factor
void view::zoom(double factor)
{
	y_extent_at_focus *= factor;
}

int view::get_modelview_projection_window_matrices(int x, int y, int width, int height,
	const dmat4** DPV_pptr,
	const dmat4** DPV_other_pptr, int* x_other_ptr, int* y_other_ptr,
	int* vp_col_idx_ptr, int* vp_row_idx_ptr,
	int* vp_width_ptr, int *vp_height_ptr,
	int* vp_center_x_ptr, int* vp_center_y_ptr,
	int* vp_center_x_other_ptr, int* vp_center_y_other_ptr) const
{
	return 0;
}
//! given a pixel location x,y return the z-value from the depth buffer, which ranges from 0.0 at z_near to 1.0 at z_far and a point in world coordinates
/*! in case of stereo rendering two z-values exist that can be unprojected to two points in world
coordinates. In this case the possibility with smaller z value is selected. */
double view::get_z_and_unproject(context& ctx, int x, int y, dvec3& p)
{
	return 0.0;
}

double view::get_z_and_unproject(context& ctx, int x, int y, vec3& p) 
{
	dvec3 dp(p);
	double res = get_z_and_unproject(ctx, x, y, dp);
	p = dp;
	return res;
}

/// fill the given vector with four points covering the screen rectangle
void view::compute_screen_rectangle(std::vector<dvec3>& rect, double depth, double aspect) const
{
	// compute view aligned coordinate system
	dvec3 x, y, z;
	put_coordinate_system(x, y, z);

	// compute center of screen covering rectangle
	dvec3 c = get_eye() - z*depth;

	// scale x- and y-direction vectors to cover screen rectangle
	double y_scale = 0.5*get_y_extent_at_depth(depth, true);
	y *= y_scale;
	x *= y_scale*aspect;

	// construct rectangle corners
	rect.push_back(c + x + y);
	rect.push_back(c - x + y);
	rect.push_back(c - x - y);
	rect.push_back(c + x - y);
}

void view::compute_screen_rectangle(std::vector<vec3>& rect, double depth, double aspect) const
{
	std::vector<dvec3> drect;
	compute_screen_rectangle(drect, depth, aspect);
	rect.clear();
	for (const auto& p : drect)
		rect.push_back(vec3(p));
}


	}
}