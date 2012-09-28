#pragma once


#include <cgv/render/view.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <cgv/math/transformations.h>
#include <cgv/math/inv.h>


/**
* A view interactor to control the opengl camera via mouse and keyboard.
*
* It parameterizes the Projection-Matrix P like gluPerspective with
* P(fovy,aspect,znear,zfar) and the ModelView-Matrix V with
* V(elevation, azimut, target, distance).
* 
* To transform a point from model coordinates x_m to camera coordinates x_c
* the following formula can be used:
*
*   x_c =  V(elevation, azimut, target, distance)* x_m
*	x_c =  T((0,0,-distance)^T)* R_x(elevation) * R_y(azimut)* T(-target)* x_m
*
* where R_x/y(ang) is a rotation around the x/y-Axis by "ang" degrees
* and T(t) is a translation by the vector t
*
*/



class spherical_view_interactor : 
	public cgv::base::node, 
	public cgv::gui::event_handler, 
	public cgv::render::drawable

{
public:
	/// constructor
	spherical_view_interactor(const char* name);
	/// return the type name 
	std::string get_type_name() const;
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	/// initialize
	bool init(cgv::render::context& ctx);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	/// this method is called in one pass over all drawables before the draw method
	void draw(cgv::render::context&);
	
	///set default values
	void set_default_values();
	
	///return projection matrix (like glu perspective)
	const cgv::math::mat<float> get_projection() const;
	/// return modelview matrix V=T((0,0,-distance)^T)* R_x(elevation) * R_y(azimut)* T(-target)
	const cgv::math::mat<float> get_modelview() const;
	/// return up dir of camera in world coords
	const cgv::math::vec<float> get_up_dir() const;	
	/// return right dir of camera in world coords
	const cgv::math::vec<float> get_right_dir()	const;
	
		
	/// add delta to azimut angle
	void rotate_azimut(float delta);
	/// add delta to elevation angle
	void rotate_elevation(float delta);
	/// add dx* left_dir + dy*up_dir to target
	void move(float dx, float dy);

private:
	/// viewport
	unsigned vp_x, vp_y, vp_width, vp_height;
	/// target point of camera
	cgv::math::vec<float>  target;
	/// elevation and azimut angle in degree
	float elevation, azimut;
	/// distance between eye and target
	float distance;


	/// near and far clipping distance
	float znear,zfar;
	///aspect (width/height)
	float aspect;
	///fov angle in y direction
	float fovy;
	
};
