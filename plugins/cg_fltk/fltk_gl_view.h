#pragma once

#include <cgv/base/group.h>
#include <cgv/signal/signal.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/utils/stopwatch.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_gl/gl/gl_performance_monitor.h>
#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/GlWindow.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif
#include "fltk_base.h"

#include "lib_begin.h"

class CGV_API fltk_gl_view : 
	public fltk::GlWindow,
	public fltk_base,
	public cgv::base::group,
	public cgv::gui::event_handler,
	public cgv::gui::provider,
	public cgv::render::gl::gl_context,
	public cgv::render::gl::gl_performance_monitor
{
protected:
	friend class fltk_driver;

	void process_text_1(const std::string& text);
	void configure_opengl_controls();

	/**@name fltk interface*/
	//@{
private:
	// opengl version used for ui only
	int version;
	cgv::gui::mouse_event dnd_release_event;
	bool dnd_release_event_queued;
	bool instant_redraw;
public:
	/// construct application
	fltk_gl_view(int x, int y, int w, int h, const std::string& name = "");
	///
	std::string get_type_name() const { return "fltk_gl_view"; }
	///
	cgv::base::group* get_group_interface();
	///
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// returns the property declaration
	std::string get_property_declarations();
	///
	void on_set(void* member_ptr);
	/// abstract interface for the setter 
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter 
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// complete drawing method that configures opengl whenever the context has changed, initializes the current frame and draws the scene
	void draw();
	/// dispatch a cgv event to the children
	bool dispatch_event(const cgv::gui::event& e);
	/// process focus and key press and release events here
	int handle(int event);
	//@}

	/**@name base interface*/
	//@{
	/// overload to show the content of this object
	void stream_stats(std::ostream& os);
	//@}

	/**@name group interface*/
	//@{
	/// helper method to remove a child
	void clear_child(cgv::base::base_ptr child);
	/// this signal is emitted when a child is removed
	cgv::signal::signal<cgv::base::base_ptr> on_remove_child;
	/// append child and return index of appended child
	unsigned int append_child(cgv::base::base_ptr child);
	/// remove all elements of the vector that point to child, return the number of removed children
	unsigned int remove_child(cgv::base::base_ptr child);
	/// remove all children
	void remove_all_children();
	/// insert a child at the given position
	void insert_child(unsigned int i, cgv::base::base_ptr child);
	/// put the event focus on the given child 
	bool set_focus(cgv::base::base_ptr child);
	/// return the currently focused child or an empty base ptr if no child has focus
	cgv::base::base_ptr get_focus() const;
	//@}

	/**@name gui interface*/
	//@{
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	///  
	void create_gui();
	//@}
public:
	/**@name context interface*/
	//@{
	void change_mode(int m);
private:
	/// store whether we are in the draw method
	bool in_draw_method;
	///
	bool redraw_request;
	///
	static void idle_callback(void* gl_view);
public:
	void create();
	void destroy();
	/// return the width of the window
	unsigned int get_width() const;
	/// return the height of the window
	unsigned int get_height() const;
	/// resize the context to the given dimensions
	void resize(unsigned int width, unsigned int height);
	/// return whether the context is currently in process of rendering
	bool in_render_process() const;
	/// overload render pass to perform measurements
	void render_pass(cgv::render::RenderPass render_pass, cgv::render::RenderPassFlags render_pass_flags, void* user_data = 0);
	/// return whether the context is created
	bool is_created() const;
	/// return whether the context is current
	bool is_current() const;
	/// return the fltk mode determined from the context_config members
	int determine_mode();
	/// set the context_config members from the current fltk mode
	void synch_with_mode();
	/// recreate context based on current context config settings
	bool recreate_context();
	/// make the current context current if possible
	bool make_current() const;
	/// clear current context lock
	void clear_current() const;
	/// attach or detach (\c attach=false) an alpha buffer to the current frame buffer if not present
	void attach_alpha_buffer(bool attach = true);
	/// attach or detach (\c attach=false) depth buffer to the current frame buffer if not present
	void attach_depth_buffer(bool attach = true);
	/// attach or detach (\c attach=false) stencil buffer to the current frame buffer if not present
	void attach_stencil_buffer(bool attach = true);
	/// return whether the graphics card supports stereo buffer mode
	bool is_stereo_buffer_supported() const;
	/// attach or detach (\c attach=false) stereo buffer to the current frame buffer if not present
	void attach_stereo_buffer(bool attach = true);
	/// attach or detach (\c attach=false) accumulation buffer to the current frame buffer if not present
	void attach_accumulation_buffer(bool attach = true);
	/// attach or detach (\c attach=false) multi sample buffer to the current frame buffer if not present
	void attach_multi_sample_buffer(bool attach = true);
	/// the context will be redrawn when the system is idle again
	void post_redraw();
	/// the context will be redrawn right now. This method cannot be called inside the following methods of a drawable: init, init_frame, draw, finish_draw
	void force_redraw();
	/// enable the given font face with the given size in pixels
	void enable_font_face(cgv::media::font::font_face_ptr font_face, float font_size);
	/// enable phong shading with the help of a shader (enabled by default)
	void enable_phong_shading();
	/// disable phong shading
	void disable_phong_shading();
	/// return whether the graphics card supports quad buffer mode
	bool is_quad_buffer_supported() const;
	//@}
private:
	bool in_recreate_context;
	bool no_more_context;
	bool started_frame_pm;
	///
	int last_mouse_x, last_mouse_y;
	/// remember the last active context to detect context changes
	void* last_context;
	/// remember the last size of the window to detect resize events
	unsigned int last_width, last_height;
};

typedef cgv::data::ref_ptr<fltk_gl_view> fltk_gl_view_ptr;

#ifdef WIN32
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<fltk_gl_view>;
#endif

#include <cgv/config/lib_end.h>
