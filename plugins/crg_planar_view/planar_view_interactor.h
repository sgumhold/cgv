#pragma once


#include <cgv/render/view.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>


/**
* A view interactor to control the opengl camera in 2d via mouse and keyboard.
*/



class planar_view_interactor : 
	public cgv::base::node, 
	public cgv::gui::event_handler, 
	public cgv::render::drawable,
	public cgv::gui::provider

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
	///
	void init_frame(cgv::render::context& ctx);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	/// this method is called in one pass over all drawables before the draw method
	void draw(cgv::render::context&);
	/// 
	void after_finish(cgv::render::context&);

	///set default values
	void set_default_values();
	///return projection matrix (like glu perspective)
	const dmat4 get_projection() const;
	/// return modelview matrix V=T((0,0,-distance)^T)* R_x(elevation) * R_y(azimut)* T(-target)
	const dmat4 get_modelview() const;

	/// describe members
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// default callback
	void on_set(void* member_ptr);
	/// create a gui
	void create_gui();
	
	void rotate(int x, int y);
	/// move (dx,dy)
	void move(int x, int y);

	void zoom(int x, int y, float ds);

private:
	float aspect;
	bool pressed;
	/// target point of camera
	vec2 target;
	vec2 pos_down;
	dmat4 MPW;

	/// zoom 
	float magnification;
	
	/// rotation angle
	float angle;

	/// whether to lock rotation
	bool lock_rotation;
	
};
