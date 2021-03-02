#pragma once

#include <cgv/base/node.h>
#include <cgv/media/font/font.h>
#include <cgv/gui/event_handler.h>
#include <cgv/render/drawable.h>

/// example for the implementation of a cgv node that handles events and renders a cube
class mouse_tracker : 
	public cgv::base::node,          /// derive from node to integrate into global tree structure and to store a name
	public cgv::gui::event_handler,  /// derive from handler to receive events and to be asked for a help string
	public cgv::render::drawable     /// derive from drawable for drawing the cube
{
protected:
	/// store selected font
	cgv::media::font::font_face_ptr ff;
	/// index into font name list
	unsigned int font_idx;
	/// store font size
	float font_size;
	/// whether font should be bold
	bool bold;
	/// whether font should be italic
	bool italic;
	/// store last mouse location
	int x,y;
	/// store list of font names
	std::vector<const char*> font_names;
	///
	cgv::math::fvec<double, 3> picked_point;
	///
	bool have_picked_point;
	///
	cgv::render::view* view_ptr;
public:
	/// construct from name which is necessary construction argument to node
	mouse_tracker(const char* name);
	/// find view ptr
	bool init(cgv::render::context& ctx);
	/// necessary method of event_handler
	bool handle(cgv::gui::event& e);
	/// show internal values
	void stream_stats(std::ostream& os);
	/// necessary method of event_handler
	void stream_help(std::ostream& os);
	/// this method is called in one pass over all drawables after drawing
	void finish_frame(cgv::render::context& ctx);
};
