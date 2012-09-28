#pragma once

#include <cgv/base/group.h>
#include <cgv/base/bool_signal.h>
#include <cgv/base/register.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/render/gl/gl_context.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

class CGV_API win32_gl_view : 
	public cgv::base::group,
	public event_handler,
	public cgv::base::registration_listener,
	public cgv::render::gl::gl_context
{
private:
	bool stereo_support;
	/// store whether we are in the draw method
	bool in_draw_method;
	///
	bool redraw_request;
	///
	int last_mouse_x, last_mouse_y;
	bool recreate_context;
	bool no_more_context;
	void* data_ptr;
	std::vector<cgv::base::base_ptr> factories;
	std::vector<char> shortcuts;
protected:
	void process_text_1(const std::string& text);
public:
	win32_gl_view(const std::string& name);
	cgv::base::bool_signal<unsigned,unsigned,long> on_new_message;
	cgv::base::bool_signal<unsigned,unsigned,long> on_unhandled_message;
	void attach(void* window_handle);
	void detach();
	void construct_context();
	void destroy_context();

	bool handle(event& e);
	void stream_help(std::ostream& os);
	bool dispatch_key_event(KeyAction ka, Keys k);
	bool dispatch_mouse_event(short x, short y, MouseAction ma, int win32_state, 
		                      unsigned char button, short dx = 32683, short dy = 32683);
	///
	cgv::base::group* get_group_interface();
	///
	void self_reflect(cgv::base::self_reflection_handler& srh);
	/// returns the property declaration
	std::string get_property_declarations();
	///
	void on_set(void* member_ptr);
	/// abstract interface for the setter 
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter 
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	///
	void* get_data();
public:
	void register_object(cgv::base::base_ptr object, const std::string& options);
	void unregister_object(cgv::base::base_ptr object, const std::string& options);

	/**@name group interface*/
	//@{
	/// helper method to remove a child
	void clear_child(cgv::base::base_ptr child);
	/// this signal is emitted when a child is removed
	cgv::base::signal<cgv::base::base_ptr> on_remove_child;
	/// append child and return index of appended child
	unsigned int append_child(cgv::base::base_ptr child);
	/// remove all elements of the vector that point to child, return the number of removed children
	unsigned int remove_child(cgv::base::base_ptr child);
	/// remove all children
	void remove_all_children();
	/// insert a child at the given position
	void insert_child(unsigned int i, cgv::base::base_ptr child);
	//@}

public:
	/// return whether the context is currently in process of rendering
	bool in_render_process() const;
	/// return whether the context is current
	bool is_current() const;
	/// make the current context current
	bool make_current() const;
	/// return the width of the window
	unsigned int get_width() const;
	/// return the height of the window
	unsigned int get_height() const;
	/// resize the context to the given dimensions
	void resize(unsigned int width, unsigned int height);
	/// the context will be redrawn when the system is idle again
	void post_redraw();
	/// the context will be redrawn right now. This method cannot be called inside the following methods of a drawable: init, init_frame, draw, finish_draw
	void force_redraw();
	/// enable the given font face with the given size in pixels
	void enable_font_face(cgv::media::font::font_face_ptr font_face, float font_size);
};
	}
}

#include <cgv/config/lib_end.h>