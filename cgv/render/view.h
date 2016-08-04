#pragma once

#include <cgv/math/fvec.h>
#include <cgv/math/mat.h>
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
	typedef cgv::math::fvec<double,3> pnt_type;
	typedef cgv::math::fvec<double,3> vec_type;
protected:
	/// focus of the view
	pnt_type focus;
	/// 
	vec_type view_up_dir;
	///
	vec_type view_dir;
	///
	double y_view_angle;
	///
	double y_extent_at_focus;
public:
	/// construct a parallel view with focus in the world origin looking in negative z-direction and the y-direction as up-direction with an extent of +-1
	view();
	/**@name getter and setter methods*/
	//@{
	///
	pnt_type get_focus() const;
	///
	vec_type get_view_up_dir() const;
	///
	vec_type get_view_dir() const;
	///
	double get_y_view_angle() const;
	///
	double get_y_extent_at_focus() const;
	/// 
	virtual void set_focus(const pnt_type& foc);
	void set_focus(double x, double y, double z);
	void set_focus(const double* foc);
	///
	virtual void set_view_up_dir(const vec_type& vud);
	void set_view_up_dir(double x, double y, double z);
	void set_view_up_dir(const double* vud);
	///
	virtual void set_view_dir(const vec_type& vd);
	void set_view_dir(double x, double y, double z);
	void set_view_dir(const double* vd);
	///
	virtual void set_y_extent_at_focus(double ext);
	///
	virtual void set_y_view_angle(double angle);
	//@}

	/**@name derived quantities*/
	//@{
	/// return whether the y view angle is zero
	bool is_parallel() const;
	/// return the eye point, in case of view angles <= 0.1 (e.g. orthographic views) the view_angle 0.1 is used to compute the eye position
	const pnt_type get_eye() const;
	/// set the view dir and angle such that get_eye() returns the passed point. This does not work in case that the eye point is identical to the focus point.
	void set_eye_keep_extent(const pnt_type& eye);
	/// set the view dir and y-extent at focus such that get_eye() returns the passed point. This does not work in case that the eye point is identical to the focus point. If the current view angle is < 0.1, the view anlge 0.1 is used for eye point calculation
	void set_eye_keep_view_angle(const pnt_type& eye);
	/// set the view according to the standard view lookat definition from eye, focus and view up direction keeping the y-extent at the focus point constant
	void view_look_at_keep_view_angle(const pnt_type& eye, const pnt_type& foc, const vec_type& vud);
	/// set the view according to the standard view lookat definition from eye, focus and view up direction keeping the y-view angle constant
	void view_look_at_keep_extent(const pnt_type& eye, const pnt_type& foc, const vec_type& vud);
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
	virtual void activate_split_viewport(cgv::render::context& ctx, unsigned col_index, unsigned row_index);
	/// deactivate the previously split viewport
	virtual void deactivate_split_viewport();
	//! given a mouse location and the pixel extent of the context, return the DPV matrix for unprojection
	/*! In stereo modes with split viewport, the returned DPV is the one the mouse pointer is on.
	The return value is in this case -1 or 1 and tells if DPV corresponds to the left (-1) or right (1) viewport.
	Furthermore, the DPV of the corresponding mouse location in the other eye is returned through DPV_other_ptr
	and the mouse location in x_other_ptr and y_other_ptr. In anaglyph or quad buffer stereo mode the other
	mouse location is identical to the incoming x and y location and 0 is returned. In mono mode,
	the other DPV and mouse locations are set to values identical to DPV and x,y and also 0 is returned.

	In case the viewport splitting was enabled during the last drawing process, the DPV and
	DPV_other matrices are set to the one valid in the panel that the mouse position x,y is
	in. The panel column and row indices are passed to the vp_col_idx and vp_row_idx pointers.
	In case that viewport splitting was disabled, 0 is passed to the panel location index pointers.

	Finally, the vp_width and vp_height pointers are set to the viewport size of a single panel
	from split stereo rendering and viewport splitting.

	All pointer arguments starting with DPV_other_ptr can be set to the null pointer.*/
	virtual int get_DPVs(int x, int y, int width, int height,
		cgv::math::mat<double>** DPV_pptr,
		cgv::math::mat<double>** DPV_other_pptr = 0, int* x_other_ptr = 0, int* y_other_ptr = 0,
		int* vp_col_idx_ptr = 0, int* vp_row_idx_ptr = 0,
		int* vp_width_ptr = 0, int *vp_height_ptr = 0);
	//! given a pixel location x,y return the z-value from the depth buffer, which ranges from 0.0 at z_near to 1.0 at z_far and a point in world coordinates
	/*! in case of stereo rendering two z-values exist that can be unprojected to two points in world
	coordinates. In this case the possibility with smaller z value is selected. */
	virtual double get_z_and_unproject(cgv::render::context& ctx, int x, int y, pnt_type& p);
	//@}

};
	}
}

#include <cgv/config/lib_end.h>