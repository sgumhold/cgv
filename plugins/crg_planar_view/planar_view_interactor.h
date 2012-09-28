#pragma once


#include <cgv/render/view.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/math/vec.h>
#include <cgv/math/mat.h>
#include <cgv/math/transformations.h>
#include <cgv/math/inv.h>


/**
* A view interactor to control the opengl camera in 2d via mouse and keyboard.
*/



class planar_view_interactor : 
	public cgv::base::node, 
	public cgv::gui::event_handler, 
	public cgv::render::drawable

{
public:
	/// constructor
	planar_view_interactor(const char* name);
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
	
	/// move (dx,dy)
	void move(int x, int y);

	void zoom(int x, int y, float ds);

private:
	/// viewport
	unsigned vp_x, vp_y, vp_width, vp_height;
	
	float aspect;
	bool pressed;
	/// target point of camera
	cgv::math::vec<float> target;
	cgv::math::vec<float> pos_down;
	cgv::math::mat<float> DPV;

	/// zoom 
	float magnification;
	
	
	
};
