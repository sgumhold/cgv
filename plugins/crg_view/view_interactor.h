#pragma once

#include <cgv/render/gl/gl_view.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/event_handler.h>

class ext_view : public cgv::render::gl::gl_view
{
public:
	ext_view();
	void set_default_values();
	void put_coordinate_system(vec_type& x, vec_type& y, vec_type& z) const;
};

#include "lib_begin.h"

class CGV_API view_interactor : 
	public cgv::base::node, 
	public cgv::gui::event_handler, 
	public cgv::render::drawable,
	public ext_view
{
public:
	///
	view_interactor(const char* name);
	/// return the type name 
	std::string get_type_name() const;
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	bool init(cgv::render::context& ctx);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context&);
	///
	void roll(double angle);
	///
	void rotate_image_plane(double ax, double ay);
private:
	double check_for_click;
	cgv::render::context::mat_type DPV;
};

#include <cgv/config/lib_end.h>
