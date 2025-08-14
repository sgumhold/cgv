#pragma once


#include <cgv/render/view.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>


class planar_view
{
	cgv::dvec2 focus;
	cgv::dvec2 center;
	double y_extent;
	double rotation;
	planar_view();
	void set_default_values();
	/// return model view transformation matrix
	cgv::dmat4 get_modelview_matrix() const;
	/// return orthogonal projection matrix based on the given aspect ratio
	cgv::dmat4 get_projection_matrix(double aspect) const;
	/// rotate view around focus by angle given in radians
	void rotate(double angle);
	/// move in screen x and screen y directions by given step lengths in world coordinates
	void pan(double step_x, double step_y);
	/// zoom by given factor
	void zoom(double factor);
};


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
	/// overload to set aspect ratio
	void resize(unsigned w, unsigned h);
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
	const cgv::dmat4 get_projection() const;
	/// return modelview matrix V=T((0,0,-distance)^T)* R_x(elevation) * R_y(azimut)* T(-target)
	const cgv::dmat4 get_modelview() const;

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
	cgv::vec2 target;
	cgv::vec2 pos_down;
	cgv::dmat4 MPW;

	/// zoom 
	float magnification;
	
	/// rotation angle
	float angle;

	/// whether to lock rotation
	bool lock_rotation;
	
};
