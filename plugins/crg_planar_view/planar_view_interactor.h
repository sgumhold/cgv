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
	cgv::dvec2 get_center() const { return center; }
	/// set the center of the view
	void set_center(const cgv::dvec2& _center);
	/// return the magnification
	double get_zoom_factor() const { return zoom_factor; }
	/// set the magnification
	void set_zoom_factor(double _zoom_factor);
	/// return the rotation angle
	double get_angle() const { return angle; }
	/// set the rotation angle
	void set_angle(double _angle);
	/// return true if the view rotation is locked
	bool is_rotation_locked() const { return lock_rotation; }
	/// set whether the rotation should be locked
	void keep_rotation_locked(bool lock);

private:
	/// width of view in pixels
	unsigned width;	
	/// height of view in pixels
	unsigned height;
	/// the aspect ratio of the viewport
	float aspect = 1.0f;
	/// point of plane shown at center of view
	cgv::dvec2 center = { 0.0, 0.0 };
	/// zoom factor with one corresponding to y-extent of 1.0
	double zoom_factor = 1.0;
	/// rotation angle
	double angle = 0.0;
	/// whether to lock rotation
	bool lock_rotation = false;

	cgv::dvec2 pixel2world(const cgv::ivec2& pixel) const;
	cgv::ivec2 world2pixel(const cgv::dvec2& point) const;
	void put_coordinate_system(cgv::dvec2& xdir, cgv::dvec2& ydir) const;

	bool pressed = false;
	cgv::dvec2 pos_down = { 0.0 };
	cgv::dmat4 MPW = { 0.0 };
};
