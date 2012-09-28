#pragma once

#include <cgv/math/fvec.h>
#include "lib_begin.h"

namespace cgv {
	namespace render {


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
	/// return the eye point, which is only valid if the view is not parallel
	const pnt_type get_eye() const;
	/// set the view dir and angle such that get_eye() returns the passed point. This does not work in case that the eye point is identical to the focus point.
	void set_eye_keep_extent(const pnt_type& eye);
	/// set the view dir and y-extent at focus such that get_eye() returns the passed point. This does not work in case that the eye point is identical to the focus point.
	void set_eye_keep_view_angle(const pnt_type& eye);
	/// set the view according to the standard view lookat definition from eye, focus and view up direction keeping the y-extent at the focus point constant
	void view_look_at_keep_view_angle(const pnt_type& eye, const pnt_type& foc, const vec_type& vud);
	/// set the view according to the standard view lookat definition from eye, focus and view up direction keeping the y-view angle constant
	void view_look_at_keep_extent(const pnt_type& eye, const pnt_type& foc, const vec_type& vud);
	//@}
};
	}
}

#include <cgv/config/lib_end.h>