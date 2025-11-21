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
	planar_view_interactor(const std::string& name);
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
	/// 
	void after_finish(cgv::render::context&);

	/// reset view to default values
	void reset_view();
	/// return projection matrix (like glu perspective)
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

	/// return the center of the view
	cgv::vec2 get_center() const { return target; }
	/// set the center of the view
	void set_center(const cgv::vec2& center);
	/// return the magnification
	float get_magnification() const { return magnification; }
	/// set the magnification
	void set_magnification(float magnification);
	/// return the rotation angle
	float get_angle() const { return angle; }
	/// set the rotation angle
	void set_angle(float angle);
	/// return true if the view rotation is locked
	bool is_rotation_locked() const { return lock_rotation; }
	/// set whether the rotation should be locked
	void keep_rotation_locked(bool lock);

private:
	/// the aspect ratio of the viewport
	float aspect = 1.0f;
	/// target point of camera
	cgv::vec2 target = { 0.0f };
	/// zoom 
	float magnification = 1.0f;;
	/// rotation angle
	float angle = 0.0f;
	/// whether to lock rotation
	bool lock_rotation = false;

	bool pressed = false;
	cgv::vec2 pos_down = { 0.0f };
	cgv::dmat4 MPW = { 0.0f };
};
