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
view::pnt_type& view::ref_focus() { return focus; }
/// write access to view up direction
view::vec_type& view::ref_view_up_dir() { return view_up_dir; }
/// write access to view dir
view::vec_type& view::ref_view_dir() { return view_dir; }
/// write access to view angle
double& view::ref_y_view_angle() { return y_view_angle; }
/// write access to extent at focus
double& view::ref_y_extent_at_focus() { return y_extent_at_focus; }

/// compute axis and angle of a rotation that the current view_dir and view_up_dir to the given target_view_dir and target_view_up_dir
int view::compute_axis_and_angle(const vec_type& target_view_dir, const vec_type& target_view_up_dir, vec_type& axis, double& angle)
{
	cgv::math::fmat<double, 3, 3> R = cgv::math::build_orthogonal_frame(view_dir, view_up_dir);
	R.transpose();
	R = cgv::math::build_orthogonal_frame(target_view_dir, target_view_up_dir)*R;
	return cgv::math::decompose_rotation_to_axis_and_angle(R, axis, angle);
}

/// compute tan of half of y field of view angle
double view::get_tan_of_half_of_fovy() const
{
	return tan(.008726646260*(y_view_angle <= 0.01 ? 0.01 : y_view_angle));
}

///
view::pnt_type view::get_focus() const
{
	return focus;
}
///
view::vec_type view::get_view_up_dir() const
{
	return view_up_dir;
}
///
view::vec_type view::get_view_dir() const
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

///
double view::get_y_extent_at_depth(double depth) const
{
	return 2.0*depth*get_tan_of_half_of_fovy();
}

/// 
void view::set_focus(const pnt_type& foc) 
{
	focus = foc;
}
void view::set_focus(double x, double y, double z) { set_focus(pnt_type(x,y,z)); }
void view::set_focus(const double* foc) { set_focus(pnt_type(foc)); }

///
void view::set_view_up_dir(const vec_type& vud)
{
	view_up_dir = vud;
}
void view::set_view_up_dir(double x, double y, double z) { set_view_up_dir(vec_type(x,y,z)); }
void view::set_view_up_dir(const double* vud) { set_view_up_dir(vec_type(vud)); }
///
void view::set_view_dir(const vec_type& vd)
{
	view_dir = vd;
}
void view::set_view_dir(double x, double y, double z) { set_view_dir(vec_type(x,y,z)); }
void view::set_view_dir(const double* vd) { set_view_dir(vec_type(vd)); }
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
/// return the eye point, which is only valid if the view is not parallel
const view::pnt_type view::get_eye() const
{
	return focus - (0.5*y_extent_at_focus / get_tan_of_half_of_fovy())*view_dir;
}

/// set the view dir and y-extent at focus such that get_eye() returns the passed point. This does not work in case that the eye point is identical to the focus point.
void view::set_eye_keep_view_angle(const pnt_type& eye)
{
	view_dir = focus-eye;
	y_extent_at_focus = get_y_extent_at_depth(view_dir.length());
	view_dir.normalize();
}

/// set the view dir and angle such the get_eye() returns the passed point. This does not work in case that the eye point is identical to the focus point.
void view::set_eye_keep_extent(const pnt_type& eye)
{
	view_dir = focus-eye;
	y_view_angle = atan(0.5*y_extent_at_focus/view_dir.length())*114.5915590;
	view_dir.normalize();
}

/// set the view according to the standard view lookat definition from eye, focus and view up direction.
void view::view_look_at_keep_extent(const pnt_type& e, const pnt_type& foc, const vec_type& vud)
{
	set_focus(foc);
	set_view_up_dir(vud);
	set_eye_keep_extent(e);
}

/// set the view according to the standard view lookat definition from eye, focus and view up direction.
void view::view_look_at_keep_view_angle(const pnt_type& e, const pnt_type& foc, const vec_type& vud)
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
void view::activate_split_viewport(cgv::render::context& ctx, unsigned col_index, unsigned row_index)
{}
/// deactivate the previously split viewport
void view::deactivate_split_viewport()
{}

/// make a viewport manage its own view
void view::viewport_use_individual_view(unsigned col_index, unsigned row_index)
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

void view::put_coordinate_system(vec_type& x, vec_type& y, vec_type& z) const
{
	z = -view_dir;
	z.normalize();
	x = cross(view_up_dir, z);
	x.normalize();
	y = cross(z, x);
}



int view::get_DPVs(int x, int y, int width, int height,
	cgv::math::mat<double>** DPV_pptr,
	cgv::math::mat<double>** DPV_other_pptr, int* x_other_ptr, int* y_other_ptr,
	int* vp_col_idx_ptr, int* vp_row_idx_ptr,
	int* vp_width_ptr, int *vp_height_ptr,
	int* vp_center_x_ptr, int* vp_center_y_ptr,
	int* vp_center_x_other_ptr, int* vp_center_y_other_ptr)
{
	return 0;
}
//! given a pixel location x,y return the z-value from the depth buffer, which ranges from 0.0 at z_near to 1.0 at z_far and a point in world coordinates
/*! in case of stereo rendering two z-values exist that can be unprojected to two points in world
coordinates. In this case the possibility with smaller z value is selected. */
double view::get_z_and_unproject(cgv::render::context& ctx, int x, int y, pnt_type& p)
{
	return 0.0;
}

/// fill the given vector with four points covering the screen rectangle
void view::compute_screen_rectangle(std::vector<pnt_type>& rect, double depth, double aspect) const
{
	// compute view aligned coordinate system
	vec_type x, y, z;
	put_coordinate_system(x, y, z);

	// compute center of screen covering rectangle
	pnt_type c = get_eye() - z*depth;

	// scale x- and y-direction vectors to cover screen rectangle
	double y_scale = 0.5*get_y_extent_at_depth(depth);
	y *= y_scale;
	x *= y_scale*aspect;

	// construct rectangle corners
	rect.push_back(c + x + y);
	rect.push_back(c - x + y);
	rect.push_back(c - x - y);
	rect.push_back(c + x - y);
}


	}
}