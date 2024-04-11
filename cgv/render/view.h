#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/fmat.h>
#include <cgv/defines/deprecated.h>
#include "lib_begin.h"

namespace cgv {
	namespace render {

		class context;

/** defines a symmetric view with the following quantities:

    - focus             ... world location of interest which projects to the center of the view
    - view direction    ... view direction in the center of the view given in world coordinates
	- view up direction ... upward direction in world coordinates that defines the y-direction
	- y_view_angle      ... opening angle of the view in y-direction
	- y_extent_at_focus ... world extent in y-direction at the focus which fills the view completely
*/
class CGV_API view
{
public:
	typedef cgv::math::fvec<float, 3>      vec3;
	typedef cgv::math::fvec<double, 3>    dvec3;
	typedef cgv::math::fmat<double, 3, 3> dmat3;
	typedef cgv::math::fmat<double, 4, 4> dmat4;
protected:
	/// focus of the view
	dvec3 focus;
	/// 
	dvec3 view_up_dir;
	///
	dvec3 view_dir;
	///
	double y_view_angle;
	///
	double y_extent_at_focus;
public:
	/// construct a parallel view with focus in the world origin looking in negative z-direction and the y-direction as up-direction with an extent of +-1
	view();

	/**@name getter and setter methods of view defining parameters*/
	//@{

	/// query focus point
	const dvec3& get_focus() const;
	/// set a new focus point keeping y extent at focus and y view angle fix, such that the eye position is also changed
	virtual void set_focus(const dvec3& foc);
	void set_focus(const vec3& foc) { set_focus(dvec3(foc)); }
	/// set focus from coordinates
	void set_focus(double x, double y, double z);
	/// write access to focus point
	dvec3& ref_focus();

	/// query current view direction
	const dvec3& get_view_dir() const;
	/// set view direction without ensuring orthogonality to view up direction
	virtual void set_view_dir(const dvec3& vd);
	void set_view_dir(const vec3& vd) { set_view_dir(dvec3(vd)); }
	/// set view direction from coordinates
	void set_view_dir(double x, double y, double z);
	/// write access to view dir
	dvec3& ref_view_dir();

	/// query current view up direction
	const dvec3& get_view_up_dir() const;
	/// set view up direction without ensuring orthogonality to view direction
	virtual void set_view_up_dir(const dvec3& vud);
	void set_view_up_dir(const vec3& vud) { set_view_up_dir(dvec3(vud)); }
	/// set view up direction from coordinates
	void set_view_up_dir(double x, double y, double z);
	/// write access to view up direction
	dvec3& ref_view_up_dir();
	/// write access to extent at focus
	double& ref_y_extent_at_focus();

	/// query opening angle (degrees) of view in y-direction
	double get_y_view_angle() const;
	/// set opening angle (degrees) of view in y-direction keeping y extent at focus resulting in a dolly zoom
	virtual void set_y_view_angle(double angle);
	/// query y extent of viewing window at focus point
	double get_y_extent_at_focus() const;
	/// set y extent of viewing window at focus point keeping y view angle resulting in a zoom
	virtual void set_y_extent_at_focus(double ext);
	/// write access to view angle
	double& ref_y_view_angle();
	//@}

	/**@name derived quantities*/
	//@{
	/// compute tan of half of y view angle, if ensure_non_zero is true, replace y view angles < 0.1 with 0.1
	double get_tan_of_half_of_fovy(bool ensure_non_zero) const;
	/// get y extent of viewing window at an arbitrary depth, if ensure_non_zero is true, replace y view angles < 0.1 with 0.1
	double get_y_extent_at_depth(double depth, bool ensure_non_zero) const;
	/// return the depth of the focus point
	double get_depth_of_focus() const;
	//! construct coordinate system with z in negative view direction and x and y aligned with the right and up direction of the viewed image
	/*! If view direction and view up direction are not orthogonal, the y direction will point in the
	component of the view up direction that is orthogonal to the view direction. */
	void put_coordinate_system(dvec3& x, dvec3& y, dvec3& z) const;
	void put_coordinate_system(vec3& x, vec3& y, vec3& z) const;
	//! compute axis and angle of a rotation that the current view_dir and view_up_dir to the given target_view_dir and target_view_up_dir
	/*! returns the result of the function cgv::math::decompose_rotation_to_axis_and_angle()*/
	int compute_axis_and_angle(const dvec3& target_view_dir, const dvec3& target_view_up_dir, dvec3& axis, double& angle);
	/// return whether the y view angle is zero
	bool is_parallel() const;
	//! query the eye point, which is computed from focus, view dir, y extent at focus and y view angle
	/*! With the y view angle approaching 0, the eye point moves infinitely far away. To avoid
	    numerical problems, the eye point is computed with an y view angle no less than 0.1.*/
	const dvec3 get_eye() const;
	//! set view dir and y view angle keeping focus and y extent such that get_eye() returns the passed point, return whether this was successful.
	/*! Recomputes view up direction to make it orthogonal to view direction. 
	    In the case that the eye point is identical to the current focus point the function fails and returns false. 
		*/
	bool set_eye_keep_extent(const dvec3& eye);
	//! set the view dir and y extent at focus keeping focus and y view angle such that get_eye() returns the passed point, return whether this was successful.
	/*! Recomputes view up direction to make it orthogonal to view direction. 
		In the case that the eye point is identical to the current focus point the function fails and returns false.
		If the current view angle is < 0.1, the view anlge 0.1 is used for eye point calculation */
	bool set_eye_keep_view_angle(const dvec3& eye);
	/// set the view according to the standard view lookat definition from eye, focus and view up direction keeping the y-extent at the focus point constant
	void view_look_at_keep_view_angle(const dvec3& eye, const dvec3& foc, const dvec3& vud);
	/// set the view according to the standard view lookat definition from eye, focus and view up direction keeping the y-view angle constant
	void view_look_at_keep_extent(const dvec3& eye, const dvec3& foc, const dvec3& vud);
	//@}

	/**@name view control*/
	//@{
	/// roll view around view direction by angle
	void roll(double angle);
	//! rotated view around axis by angle
	/*! Axis is given by point and direction, where the point is in the image center and the given depth
	    and the axis points into a direction in image plane given through its screen x and screen y 
		coordinate. The length of the axis vector gives the rotation angle in radians.
		Rotation around screen x direction corresponds to yaw and around screen y direction
		to gear rotations. */
	void rotate(double axis_direction_x, double axis_direction_y, double axis_point_depth);
	/// move along view direction by given step length in world coordinates
	void move(double step);
	/// move in screen x and screen y directions by given step lengths in world coordinates
	void pan(double step_x, double step_y);
	/// zoom by given factor
	void zoom(double factor);
	//@}

	/**@name viewport splitting*/
	//@{
	/// call this function before a drawing process to support viewport splitting inside the draw call via the activate/deactivate functions
	virtual void enable_viewport_splitting(unsigned nr_cols, unsigned nr_rows);
	/// check whether viewport splitting is activated and optionally set the number of columns and rows if corresponding pointers are passed
	virtual bool is_viewport_splitting_enabled(unsigned* nr_cols_ptr = 0, unsigned* nr_rows_ptr = 0) const;
	/// disable viewport splitting
	virtual void disable_viewport_splitting();
	/// inside the drawing process activate the sub-viewport with the given column and row indices, always terminate an activated viewport with deactivate_split_viewport
	virtual void activate_split_viewport(context& ctx, unsigned col_index, unsigned row_index);
	/// deactivate the previously split viewport
	virtual void deactivate_split_viewport(context& ctx);
	/// make a viewport manage its own view
	virtual void enable_viewport_individual_view(unsigned col_index, unsigned row_index, bool enable = true);
	/// check whether viewport manage its own view
	virtual bool does_viewport_use_individual_view(unsigned col_index, unsigned row_index) const;
	/// access the view of a given viewport
	virtual view& ref_viewport_view(unsigned col_index, unsigned row_index);
	//! given an opengl pixel location and the size in pixels of the opengl window, return the modelview_projection_window matrix for unprojection
	/*! In stereo modes with split viewport, the returned MPW is the one opengl pixel location is in.
	The return value is in this case -1 or 1 and tells if MPW corresponds to the left (-1) or right (1) viewport.
	Furthermore, the MPW of the corresponding opengl pixel location in the other eye is returned through MPW_other_ptr
	and the opengl pixel location in x_other_ptr and y_other_ptr. In anaglyph or quad buffer stereo mode the other
	mouse location is identical to the incoming x and y location and 0 is returned. In mono mode,
	the other MPW and mouse locations are set to values identical to MPW and x,y and also 0 is returned.

	In case the viewport splitting was enabled during the last drawing process, the MPW and
	MPW_other matrices are set to the one valid in the panel that the opengl pixel location x,y is
	in. The panel column and row indices (counting from bottom to top as in opengl) are passed to 
	the vp_col_idx and vp_row_idx pointers.
	In case that viewport splitting was disabled, 0 is passed to the panel location index pointers.

	Finally, the vp_width, vp_height, vp_center_x, and vp_center_y pointers are set to the viewport size
	and center mouse location of the panel panel that the mouse pointer is in.

	All pointer arguments starting with MPW_other_ptr can be set to the null pointer.*/
	virtual int get_modelview_projection_window_matrices(int x, int y, int width, int height,
		const dmat4** MPW_pptr,
		const dmat4** MPW_other_pptr = 0, int* x_other_ptr = 0, int* y_other_ptr = 0,
		int* vp_col_idx_ptr = 0, int* vp_row_idx_ptr = 0,
		int* vp_width_ptr = 0, int *vp_height_ptr = 0,
		int* vp_center_x_ptr = 0, int* vp_center_y_ptr = 0,
		int* vp_center_x_other_ptr = 0, int* vp_center_y_other_ptr = 0) const;

	DEPRECATED("deprecated, use get_modelview_projection_window_matrices() instead.") virtual int get_modelview_projection_device_matrices(int x, int y, int width, int height,
		const dmat4** MVPD_pptr,
		const dmat4** MVPD_other_pptr = 0, int* x_other_ptr = 0, int* y_other_ptr = 0,
		int* vp_col_idx_ptr = 0, int* vp_row_idx_ptr = 0,
		int* vp_width_ptr = 0, int *vp_height_ptr = 0,
		int* vp_center_x_ptr = 0, int* vp_center_y_ptr = 0,
		int* vp_center_x_other_ptr = 0, int* vp_center_y_other_ptr = 0) const {
		return get_modelview_projection_window_matrices(x,y,width,height,MVPD_pptr,MVPD_other_pptr,
			x_other_ptr,y_other_ptr,vp_col_idx_ptr, vp_row_idx_ptr, vp_width_ptr, vp_height_ptr, 
			vp_center_x_ptr, vp_center_y_ptr, vp_center_x_other_ptr, vp_center_y_other_ptr);
	}
	//! given an opengl pixel location x,y return the window z-value from the depth buffer
	/*! in case of stereo rendering two z-values exist that can be unprojected to two points in world
	    coordinates. In this case the possibility with smaller z value is selected. */
	virtual double get_z_and_unproject(context& ctx, int x, int y, dvec3& p);
	double get_z_and_unproject(context& ctx, int x, int y, vec3& p);
	/// fill \c rect with four points covering the screen rectangle at given depth from eye with given aspect ratio
	void compute_screen_rectangle(std::vector<dvec3>& rect, double depth, double aspect) const;
	void compute_screen_rectangle(std::vector<vec3>& rect, double depth, double aspect) const;
	//@}

};
	}
}

#include <cgv/config/lib_end.h>