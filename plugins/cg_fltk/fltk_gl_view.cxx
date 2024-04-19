#include "fltk_gl_view.h"
#include "fltk_event.h"
#include "fltk_driver.h"
#include "fltk_font_server.h"
#include <cgv/render/drawable.h>
#include <cgv/base/register.h>
#include <cgv/utils/ostream_printf.h>
#include <libs/cgv_reflect_types/math/fvec.h>
#include <cgv/utils/scan.h>
#include <cgv/type/variant.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv_gl/gl/gl.h>

#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/draw.h>
#include <fltk/Font.h>
#include <fltk/gl.h>
#include <fltk/visual.h>
#include <iostream>

#ifdef WIN32
#include <windows.h>
#endif

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::signal;
using namespace cgv::render::gl;
using namespace cgv::utils;

void fltk_gl_view::process_text_1(const std::string& text)
{
	if (!text.empty())
		process_text(text);
}

class MyGlWindow : public fltk::Window
{
public:
	int mode_;
	fltk::GlChoice *gl_choice;
	fltk::GLContext context_;
	char valid_;
	char damage1_; // damage() of back buffer

	void *overlay;
};

fltk::GLContext get_context(const fltk::GlWindow* glw)
{
	return reinterpret_cast<const MyGlWindow*>(glw)->context_;
}


/// construct application
fltk_gl_view::fltk_gl_view(int x, int y, int w, int h, const std::string& name) 
	: fltk::GlWindow(x,y,w,h), group(name), dnd_release_event(0,0,MA_ENTER)
{
	version = -1;

	last_context = (void*)-1;
	last_width = -1;
	last_height = -1;

	in_recreate_context = false;
	no_more_context = false;
	instant_redraw = false;
	in_draw_method = false;
	redraw_request = false;

	enabled = false;
	started_frame_pm = false;

	dnd_release_event_queued = false;

	connect(out_stream.write, this, &fltk_gl_view::process_text_1);
	mode(determine_mode());
}

///
cgv::base::group* fltk_gl_view::get_group_interface()
{
	return this;
}

///
bool fltk_gl_view::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return
		context_config::self_reflect(srh) &&
		srh.reflect_member("version", version) &&
		srh.reflect_member("vsync", enable_vsync) &&
		srh.reflect_member("instant_redraw", instant_redraw) &&
		srh.reflect_member("sRGB_framebuffer", sRGB_framebuffer) &&
		srh.reflect_member("show_help", show_help) &&
		srh.reflect_member("show_stats", show_stats) &&
		srh.reflect_member("font_size", info_font_size) &&
		srh.reflect_member("tab_size", tab_size) &&
		srh.reflect_member("performance_monitoring", enabled) &&
		// TODO: test
		srh.reflect_member("bg_r", bg_color_stack.top()[0]) &&
		srh.reflect_member("bg_g", bg_color_stack.top()[1]) &&
		srh.reflect_member("bg_b", bg_color_stack.top()[2]) &&
		srh.reflect_member("bg_a", bg_color_stack.top()[3]) &&
		srh.reflect_member("bg_index", current_background) &&
		srh.reflect_member("gamma", gamma) &&
		srh.reflect_member("gamma3", gamma3) &&
		srh.reflect_member("nr_display_cycles", nr_display_cycles) &&
		srh.reflect_member("bar_line_width", bar_line_width) &&
		srh.reflect_member("file_name", file_name);
}

void fltk_gl_view::on_set(void* member_ptr)
{
	if (member_ptr == &version) {
		if (version == -1)
			version_major = version_minor = -1;
		else {
			version_major = version / 10;
			version_minor = version % 10;
		}
		member_ptr = &version_major;
	}
	if (member_ptr >= static_cast<context_config*>(this) && member_ptr < static_cast<context_config*>(this) + 1) {
		int new_mode = determine_mode();
		if (is_created() && !in_recreate_context) { // && new_mode != mode()) {
			if (can_do(new_mode))
				change_mode(new_mode);
			else
				synch_with_mode();
		}
	}
	if(member_ptr == &current_background) {
		set_bg_clr_idx(current_background);
		update_member(&bg_color_stack.top()[0]);
	}
	if (member_ptr == &gamma) {
		set_gamma(gamma);
		update_member(&gamma3[0]);
		update_member(&gamma3[1]);
		update_member(&gamma3[2]);
	}
	update_member(member_ptr);
	//if(member_ptr == &bg_color_stack.top()[1] || member_ptr == &bg_color_stack.top()[2])
	//	update_member(&bg_color_stack.top()[0]);
	//if (member_ptr == &bg_accum_color_stack.top()[1] || member_ptr == &bg_accum_color_stack.top()[2])
	//	update_member(&bg_accum_color_stack.top()[0]);
	post_redraw();
}

/// returns the property declaration
std::string fltk_gl_view::get_property_declarations()
{
	return fltk_base::get_property_declarations()+";"+cgv::base::group::get_property_declarations()+";stencil_buffer:bool;accum_buffer:bool;stereo_buffer:bool;multisample:bool";
}

void fltk_gl_view::change_mode(int m)
{
	in_recreate_context = true;
	if (mode() == m)
		mode_ = -1;
	fltk_driver::set_context_creation_attrib_list(*this);
	if (!mode(m))
		std::cerr << "could not change mode to " << m << std::endl;
	current_font_face.clear();
	current_font_size = 0;
	synch_with_mode();
	in_recreate_context = false;
	update_member(this);
}

int fltk_gl_view::determine_mode()
{
	int m = fltk::NO_AUTO_SWAP;
	if (depth_buffer)
		m = m | fltk::DEPTH_BUFFER;
	if (stereo_buffer)
		m = m | fltk::STEREO;
	if (stencil_buffer)
		m = m | fltk::STENCIL_BUFFER;
	if (alpha_buffer)
		m = m | fltk::ALPHA_BUFFER;
	if (accumulation_buffer)
		m = m | fltk::ACCUM_BUFFER;
	if (context_config::double_buffer)
		m = m | fltk::DOUBLE_BUFFER;
	if (multi_sample_buffer)
		m = m | fltk::MULTISAMPLE;
	return m;
}

/// set the context_config members from the current fltk mode
void fltk_gl_view::synch_with_mode()
{
	version = 10 * version_major + version_minor;

	update_member(&version);
	update_member(&version_major);
	update_member(&version_minor);
	update_member(&debug);
	update_member(&core_profile);
	update_member(&forward_compatible);
	configure_opengl_controls();

	int m = mode();
	make_current();
	bool new_stereo_buffer = (m & fltk::STEREO) != 0;
	if (new_stereo_buffer != stereo_buffer) {
		stereo_buffer = new_stereo_buffer;
		update_member(&stereo_buffer);
	}
	bool new_depth_buffer = (m & fltk::DEPTH_BUFFER) != 0;
	if (new_depth_buffer != depth_buffer) {
		depth_buffer = new_depth_buffer;
		update_member(&depth_buffer);
	}
	if (depth_buffer) {
		GLint depthSize = 24;
		if (version >= 30) {
			glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER,
				GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthSize);
		}
		depth_bits = depthSize;
	}
	else
		depth_bits = -1;
	update_member(&depth_bits);
	
	bool new_stencil_buffer = (m & fltk::STENCIL_BUFFER) != 0;
	if (new_stencil_buffer != stencil_buffer) {
		stencil_buffer = new_stencil_buffer;
		update_member(&stencil_buffer);
	}
	if (stencil_buffer) {
		GLint stencilSize = 8;
		if (version >= 30) {
			glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER,
				GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencilSize);
		}
		stencil_bits = stencilSize;
	}
	else
		stencil_bits = -1;
	update_member(&stencil_bits);
	
	bool new_alpha_buffer = (m & fltk::ALPHA_BUFFER) != 0;
	if (new_alpha_buffer != alpha_buffer) {
		alpha_buffer = new_alpha_buffer;
		update_member(&alpha_buffer);
	}
	bool new_multi_sample_buffer = (m & fltk::MULTISAMPLE) != 0;
	if (new_multi_sample_buffer != multi_sample_buffer) {
		multi_sample_buffer = new_multi_sample_buffer;
		update_member(&multi_sample_buffer);
	}
	if (multi_sample_buffer) {
		GLint nrSamples = 0;
		if (version >= 13)
			glGetIntegerv(GL_SAMPLES, &nrSamples);
		nr_multi_samples = nrSamples;
	}
	else
		nr_multi_samples = -1;
	update_member(&nr_multi_samples);
	
	bool new_double_buffer = (m & fltk::DOUBLE_BUFFER) != 0;
	if (new_double_buffer != context_config::double_buffer) {
		context_config::double_buffer = new_double_buffer;
		update_member(&(context_config::double_buffer));
	}
	bool new_accumulation_buffer = (m & fltk::ACCUM_BUFFER) != 0;
	if (new_accumulation_buffer != accumulation_buffer) {
		accumulation_buffer = new_accumulation_buffer;
		update_member(&accumulation_buffer);
	}
}

/// attach or detach (\c attach=false) an alpha buffer to the current frame buffer if not present
void fltk_gl_view::attach_alpha_buffer(bool attach)
{
	if (alpha_buffer != attach) {
		alpha_buffer = attach;
		on_set(&alpha_buffer);
	}
}
/// attach or detach (\c attach=false) depth buffer to the current frame buffer if not present
void fltk_gl_view::attach_depth_buffer(bool attach)
{
	if (depth_buffer != attach) {
		depth_buffer = attach;
		on_set(&depth_buffer);
	}
}
/// attach or detach (\c attach=false) stencil buffer to the current frame buffer if not present
void fltk_gl_view::attach_stencil_buffer(bool attach)
{
	if (stencil_buffer != attach) {
		stencil_buffer = attach;
		on_set(&stencil_buffer);
	}
}

/// return whether the graphics card supports stereo buffer mode
bool fltk_gl_view::is_stereo_buffer_supported() const
{
	return can_do(mode() | fltk::STEREO);
}
/// attach or detach (\c attach=false) stereo buffer to the current frame buffer if not present
void fltk_gl_view::attach_stereo_buffer(bool attach)
{
	if (stereo_buffer != attach) {
		stereo_buffer = attach;
		on_set(&stereo_buffer);
	}
}
/// attach or detach (\c attach=false) accumulation buffer to the current frame buffer if not present
void fltk_gl_view::attach_accumulation_buffer(bool attach)
{
	if (accumulation_buffer != attach) {
		accumulation_buffer = attach;
		on_set(&accumulation_buffer);
	}
}
/// attach or detach (\c attach=false) multi sample buffer to the current frame buffer if not present
void fltk_gl_view::attach_multi_sample_buffer(bool attach)
{
	if (multi_sample_buffer != attach) {
		multi_sample_buffer = attach;
		on_set(&multi_sample_buffer);
	}
}

/// attach a stencil buffer to the current frame buffer if not present
bool fltk_gl_view::recreate_context()
{
	int m = determine_mode();
	if (!can_do(m))
		return false;

	change_mode(m);
	return true;
}

#include <cgv/gui/dialog.h>

/// abstract interface for the setter 
bool fltk_gl_view::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (fltk_base::set_void(this,this,property, value_type, value_ptr))
		return true;
	return base::set_void(property, value_type, value_ptr);
}

/// abstract interface for the getter 
bool fltk_gl_view::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (fltk_base::get_void(this,this,property, value_type, value_ptr))
		return true;
	return base::get_void(property, value_type, value_ptr);
}

/// interface of a handler for traverse callbacks
class debug_traverse_callback_handler : public traverse_callback_handler
{
protected:
	int i;
	void tab() { for (int j=0; j<i; ++j) std::cout << " "; }
	std::string get_name(base_ptr b) const {
		named_ptr n = b->get_named();
		if (n && !n->get_name().empty())
			return n->get_name();
		return b->get_type_name();
	}
public:
	debug_traverse_callback_handler() : i(0) {}
	/// called before a node b is processed, return, whether to skip this node. If the node is skipped, the on_leave_node callback is still called
	bool on_enter_node(base_ptr b) { tab(); std::cout << "enter node " << get_name(b) << std::endl; ++i; return false; }
	/// called when a node b is left, return whether to terminate traversal
	bool on_leave_node(base_ptr b) { --i; tab(); std::cout << "leave node " << get_name(b) << std::endl; return false; }
	/// called before the children of a group node g are processed, return whether these should be skipped. If children are skipped, the on_leave_children callback is still called.
	bool on_enter_children(group_ptr g) { tab(); std::cout << "enter children " << get_name(g) << std::endl; ++i; return false; }
	/// called when the children of a group node g have been left, return whether to terminate traversal
	virtual bool on_leave_children(group_ptr g) { --i; tab(); std::cout << "leave children " << get_name(g) << std::endl; return false; }
	/// called before the parent of a node n is processed, return whether this should be skipped. If the parent is skipped, the on_leave_parent callback is still called.
	virtual bool on_enter_parent(node_ptr n) { tab(); std::cout << "enter parent " << get_name(n) << std::endl; ++i; return false; } 
	/// called when the parent of a node n has been left, return whether to terminate traversal
	virtual bool on_leave_parent(node_ptr n) { --i; tab(); std::cout << "leave parent " << get_name(n) << std::endl; return false; }
};

#include <fltk/../../compat/FL/Fl.H>

void fltk_gl_view::create()
{
	fltk_driver::set_context_creation_attrib_list(*this);
	fltk::GlWindow::create();
	make_current();
	last_context = get_context(this);
	if (!configure_gl()) {
		abort();
	}
	synch_with_mode();
}

void fltk_gl_view::destroy()
{
	make_current();
	destruct_render_objects();
	if (!in_recreate_context) {
		remove_all_children();
		no_more_context = true;
	}
	else {
		single_method_action<cgv::render::drawable,void,cgv::render::context&> sma(*this, &drawable::clear, true, true);
		for (unsigned i=0; i<get_nr_children(); ++i)
			traverser(sma, "nc", TS_DEPTH_FIRST, false, true).traverse(get_child(i));
	}
	//destruct_render_objects();
	fltk::GlWindow::destroy();
}

/// helper method to remove a child
void fltk_gl_view::clear_child(base_ptr child)
{
	single_method_action<cgv::render::drawable,void,cgv::render::context&> sma(*this, &drawable::clear, false, false);
	traverser(sma, "nc", TS_DEPTH_FIRST, false, true).traverse(child);

	on_remove_child(child);
}

/// append child and return index of appended child
unsigned int fltk_gl_view::append_child(base_ptr child)
{
	if (child->get_interface<drawable>() == 0)
		return -1;
	unsigned int i = group::append_child(child);
	configure_new_child(child);
	set_focused_child(i);
	return i;
}
/// remove all elements of the vector that point to child, return the number of removed children
unsigned int fltk_gl_view::remove_child(base_ptr child)
{
	clear_child(child);
	unsigned j = group::remove_child(child);
	int i = get_focused_child();
	if (i != -1) {
		if (j <= (unsigned)i) {
			if (i == 0)
				i = get_nr_children();
			set_focused_child(i-1);
		}
	}
	return j;
}
/// remove all children
void fltk_gl_view::remove_all_children()
{
	for (unsigned int i=0; i<get_nr_children(); ++i)
		clear_child(get_child(i));
	group::remove_all_children();
}
/// insert a child at the given position
void fltk_gl_view::insert_child(unsigned int i, base_ptr child)
{
	if (child->get_interface<drawable>() == 0)
		return;
	group::insert_child(i,child);
	configure_new_child(child);
	set_focused_child(i);
}

///
void fltk_gl_view::idle_callback(void* ptr)
{
	fltk_gl_view* gl_view = (fltk_gl_view*) ptr;
	gl_view->redraw();
}

/// overload render pass to perform measurements
void fltk_gl_view::render_pass(cgv::render::RenderPass rp, cgv::render::RenderPassFlags rpf, void* ud)
{
	start_task((int)rp);
	cgv::render::gl::gl_context::render_pass(rp, rpf, ud);
	if (enabled)
		glFinish();
	else
		glFlush();
	finish_task((int)rp);
}

/// complete drawing method that configures opengl whenever the context has changed, initializes the current frame and draws the scene
void fltk_gl_view::draw()
{
	if (!started_frame_pm)
		start_frame();

	in_draw_method = true;
	bool last_redraw_request = redraw_request;
	redraw_request = false;
	if (!valid()) {
		if (get_context(this) != last_context) {
			last_width = get_width();
			last_height = get_height();
			configure_gl();
			synch_with_mode();
		}
		else if ((int)last_width != get_width() || (int)last_height != get_height()) {
			last_width = get_width();
			last_height = get_height();
			resize_gl();
		}
	}
	render_pass(RP_MAIN, default_render_flags);

	if (enabled) {
		cgv::render::gl::gl_performance_monitor::draw(*this);	
		glFlush();
	}

	swap_buffers();

	if (enabled) {
		finish_frame();
		update_member(&fps);
		if (redraw_request) {
			start_frame();
			started_frame_pm = true;
		}
		else 
			started_frame_pm = false;
	}
	if (instant_redraw)
		redraw_request = true;

	in_draw_method = false;
	if (redraw_request != last_redraw_request) {
		if (redraw_request)
			fltk::add_idle(idle_callback, this);
		else
			fltk::remove_idle(idle_callback, this);
	}
}

/// put the event focus on the given child 
bool fltk_gl_view::set_focus(base_ptr child)
{
	for (unsigned int i = 0; i < get_nr_children(); ++i)
		if (child == get_child(i)) {
			set_focused_child(i);
			if (show_stats)
				redraw();
			return true;
		}
	return false;
}

/// return the currently focused child or an empty base ptr if no child has focus
cgv::base::base_ptr fltk_gl_view::get_focus() const
{
	int i = get_focused_child();
	if (i == -1)
		return base_ptr();
	return get_child(i);
}

/// handle events emitted by event sources
bool fltk_gl_view::handle(event& e)
{
	if (e.get_kind() == EID_KEY) {
		key_event& ke = static_cast<key_event&>(e);
		if (ke.get_action() == KA_PRESS) {
			switch (ke.get_key()) {
			case KEY_F4 : 
				if (ke.get_modifiers() == EM_ALT) {
					cgv::base::unregister_all_objects();
					destroy();
#ifdef WIN32
					TerminateProcess(GetCurrentProcess(),0);
#else
					exit(0);
#endif
				}
				break;
			case KEY_F1 : 
				if (ke.get_modifiers() == 0) {
					show_help = !show_help;
					redraw();
					return true;
				}
				break;
			case KEY_Delete :
				if (ke.get_modifiers() == EM_ALT && get_nr_children() > 1 && get_focused_child() != -1) {
					cgv::base::unregister_object(get_child(get_focused_child()), "");
//						remove_child(get_child(get_focused_child()));
					if (get_focused_child() >= (int)get_nr_children())
						traverse_policy::set_focused_child(get_focused_child()-1);
					redraw();
					return true;
				}
				break;
			case KEY_Tab :
				if (ke.get_modifiers() == EM_SHIFT) {
					traverse_policy::set_focused_child(get_focused_child()-1);
					if (get_focused_child() == -2)
						traverse_policy::set_focused_child((int)get_nr_children()-1);
					if (show_stats)
						redraw();
					return true;
				}
				else if (ke.get_modifiers() == 0) {
					traverse_policy::set_focused_child(get_focused_child()+1);
					if (get_focused_child() >= (int)get_nr_children())
						traverse_policy::set_focused_child(-1);
					if (show_stats)
						redraw();
					return true;
				}
				break;
			case KEY_F8 : 
				if (ke.get_modifiers() == 0) {
					show_stats = !show_stats; 
					redraw();
					return true;
				}
				if (ke.get_modifiers() == EM_CTRL) {
					enabled = !enabled; 
					redraw();
					return true;
				}
				break;
			case KEY_F10 :
				if (ke.get_modifiers() == 0) {
					do_screen_shot = true;
					redraw();
					return true;
				}
				break;
			case KEY_F12 :
				if (ke.get_modifiers() == 0) {
					set_bg_clr_idx(current_background+1);
					post_redraw();
					return true;
				}
				break;
			default: 
				return false;
			}
		}
	}
	return false;
}
/// overload to stream help information to the given output stream
void fltk_gl_view::stream_help(std::ostream& os)
{
	os << "MAIN: Show   ... help: F1;                  ... [perf-]status: [Ctrl-]F8;\n";
	os << "      Object ... change focus: [Shift-]Tab; ... remove focused: Delete\n";
	os << "      Screen ... copy to file: F10;         ... background: F12\n";
	os << "      Toggle ... menu: Menu; GUI: Shift-Key; FullScreen: F11; Monitor: Shift-F11\n";
	os << "Quit: Alt-F4\n" << std::endl;
}

void fltk_gl_view::stream_stats(std::ostream& os)
{
	std::string name("no focus");
	if (get_focused_child() != -1) {
		base_ptr c = get_child(get_focused_child());
		named_ptr n = c->get_interface<named>();
		if (!n.empty()) {
			name  = n->get_name();
			name += ':';
		}
		else
			name = "";
		name += c->get_type_name();
	}
	oprintf(os, "WxH=%d:%d focus = %s<Tab>\n", w(), h(), name.c_str());
}

/// return the width of the window
unsigned int fltk_gl_view::get_width() const
{
	return w();
}
/// return the height of the window
unsigned int fltk_gl_view::get_height() const
{
	return h();
}

/// resize the context to the given dimensions
void fltk_gl_view::resize(unsigned int width, unsigned int height)
{
	node_ptr p = get_parent();
	if (p) {
		p->set("W", width);
		p->set("H", height);
	}
	else {
		fltk::GlWindow::resize(width, height);
//		cgv::render::gl::gl_context::resize(width, height);
	}
}

/// return whether the context is currently in process of rendering
bool fltk_gl_view::in_render_process() const
{
	return in_draw_method;
}

/// return whether the context is created
bool fltk_gl_view::is_created() const
{
	return get_context(this) != 0;
}

/// return whether the context is current
bool fltk_gl_view::is_current() const
{
	return false;
}
/// make the current context current
bool fltk_gl_view::make_current() const
{
	if (no_more_context)
		return false;
	const_cast<fltk::GlWindow*>(static_cast<const fltk::GlWindow*>(this))->make_current();
	return true;
}

/// clear current context lock
void fltk_gl_view::clear_current() const
{
#if USE_X11
	std::cerr << "clear_current() not implemented for FLTK under linux!" << std::endl;
	// glXMakeCurrent(xdisplay, 0, 0);
#elif defined(_WIN32)
	wglMakeCurrent(0, 0);
#elif defined(__APPLE__)
	// warning: the Quartz version should probably use Core GL (CGL) instead of AGL
	aglSetCurrentContext(0);
#endif
}

/// the context will be redrawn when the system is idle again
void fltk_gl_view::post_redraw()
{
	if(in_render_process())
		// cannot directly issue a redraw during an active render process; instead request a redraw for next idle period
		redraw_request = true;
	else
		redraw();
}


void fltk_gl_view::force_redraw()
{
	if (in_render_process()) {
		std::cerr << "force_redraw not allowed in render process" << std::endl;
		post_redraw();
		return;
	}
	flush();
	set_damage(0);
}

/// select a font given by a font handle
void fltk_gl_view::enable_font_face(font_face_ptr font_face, float font_size)
{
	fltk_font_face_ptr fff = dynamic_cast<fltk_font_face*>(&*font_face);
	if (fff.empty())
		gl_context::enable_font_face(font_face, font_size);
	else {
		// avoid fltk fonts in core profile
		if (!core_profile)
			fff->enable(static_cast<cgv::render::context*>(this), font_size);
		current_font_face = font_face;
		current_font_size = font_size;
	}
}

bool fltk_gl_view::dispatch_event(const event& e)
{
	// FIXME: cast from (const event&) to (event&) is dirty
	single_method_action<event_handler,bool,event&> sma((event&)e,&event_handler::handle);
	return traverser(sma).traverse(group_ptr(this));
}

/// process focus and key press and release events here
int fltk_gl_view::handle(int ei)
{
	fltk_base::handle(this,ei);
	int dx = fltk::event_x() - last_mouse_x;
	int dy = fltk::event_y() - last_mouse_y;
	last_mouse_x = fltk::event_x();
	last_mouse_y = fltk::event_y();

	switch (ei) {
	case fltk::FOCUS:
	case fltk::UNFOCUS: 
		return 1;
	case fltk::KEY :
		if (dispatch_event(cgv_key_event(fltk::event_key_repeated() ? KA_REPEAT : KA_PRESS)))
			return 1;
		if (fltk::event_key() >= fltk::HomeKey &&
			fltk::event_key() <= fltk::EndKey)
			return 1;
		break;
	case fltk::KEYUP :
		if (dispatch_event(cgv_key_event(KA_RELEASE)))
			return 1;
		break;
	case fltk::PUSH:
		if (dispatch_event(cgv_mouse_event(MA_PRESS)))
			return 1;
		break;
	case fltk::RELEASE:
		if (dispatch_event(cgv_mouse_event(MA_RELEASE)))
			return 1;
		break;
	case fltk::MOUSEWHEEL:
		if (dispatch_event(cgv_mouse_event(MA_WHEEL)))
			return 1;
		break;
	case fltk::MOVE:
		if (!dnd_release_event_queued) {
			if ( (dx != 0 || dy != 0) && dispatch_event(cgv_mouse_event(MA_MOVE,dx,dy)))
				return 1;
		}
		else 
			return 1;
		break;
	case fltk::DRAG:
		if (dx != 0 || dy != 0) {
			if (dispatch_event(cgv_mouse_event(MA_DRAG,dx,dy)))
				return 1;
		}
		else
			return 1;
		break;
	case fltk::ENTER:
		take_focus();
		dispatch_event(cgv_mouse_event(MA_ENTER));
		return 1;
	case fltk::LEAVE:
		if (dispatch_event(cgv_mouse_event(MA_LEAVE)))
			return 1;
		break;
	case fltk::DND_ENTER:
	{
		cgv::gui::mouse_event me = cgv_mouse_event(MA_ENTER, EF_DND);
		me.set_dnd_text(fltk::event_text());
		if (dispatch_event(me))
			return 1;
		break;
	}
	case fltk::DND_DRAG :
		if (dx != 0 || dy != 0) {
			if (dispatch_event(cgv_mouse_event(MA_DRAG,EF_DND,dx,dy)))
				return 1;
		}
		else
			return 1;
		break;
	case fltk::DND_LEAVE:
		if (dispatch_event(cgv_mouse_event(MA_LEAVE, EF_DND)))
			return 1;
		break;
	case fltk::DND_RELEASE :
		dnd_release_event = cgv_mouse_event(MA_RELEASE, EF_DND);
		dnd_release_event_queued = true;
		return 1;
	case fltk::PASTE :
		dnd_release_event_queued = false;
		dnd_release_event.set_dnd_text(fltk::event_text());
		if (dispatch_event(dnd_release_event))
			return 1;
		break;
	}
	return fltk::GlWindow::handle(ei);       // hand on event to base class
}

void* to_void_ptr(int i) {
	void* res = 0;
	(int&) res = i;
	return res;
}
/// enable phong shading with the help of a shader (enabled by default)
void fltk_gl_view::enable_phong_shading()
{
	cgv::render::context::enable_phong_shading();
	update_member(&phong_shading);
}
/// disable phong shading
void fltk_gl_view::disable_phong_shading()
{
	gl_context::disable_phong_shading();
	update_member(&phong_shading);
}
void fltk_gl_view::configure_opengl_controls()
{
	if (find_control(debug))
		find_control(debug)->set("active", version >= 30);
	if (find_control(forward_compatible))
		find_control(forward_compatible)->set("active", version >= 30);
	if (find_control(core_profile))
		find_control(core_profile)->set("active", version >= 32);
}

///  
void fltk_gl_view::create_gui()
{
	add_decorator("GL View", "heading");
	if (begin_tree_node("Rendering", fps, false)) {
		provider::align("\a");
		add_view("FPS", fps, "", "w=72", " ");
		add_member_control(this, "EWMA", fps_alpha, "value_slider", "min=0;max=1;ticks=true;w=120;align='B';tooltip='coefficient of exponentially weighted moving average'");
		add_member_control(this, "VSync", enable_vsync, "toggle", "w=92", " ");
		add_member_control(this, "Instant Redraw", instant_redraw, "toggle", "w=100");
		add_gui("Gamma", gamma3, "vector", "gui_type='value_slider';components='RGB';options='min=0.2;max=5;ticks=true;log=true'");
		find_control(gamma3[0])->set("tooltip", "default gamma used for inverse gamma correction of red fragment channel");
		find_control(gamma3[1])->set("tooltip", "default gamma used for inverse gamma correction of green fragment channel");
		find_control(gamma3[2])->set("tooltip", "default gamma used for inverse gamma correction of blue fragment channel");
		add_member_control(this, "sRGB Framebuffer", sRGB_framebuffer, "check");
		provider::align("\b");
		end_tree_node(fps);
	}
	if (begin_tree_node("Debug", enabled, false)) {
		provider::align("\a");
		add_member_control(this, "Show Help", show_help, "check");
		add_member_control(this, "Show Stats", show_stats, "check");
		add_member_control(this, "Debug Render Passes", debug_render_passes, "check");
		add_member_control(this, "Performance Monitoring", enabled, "check");
		add_member_control(this, "Time Scale", time_scale, "value_slider", "min=1;max=90;ticks=true;log=true");
		add_gui("Placement", placement, "", "options='min=0;max=500'");
		provider::align("\b");
		end_tree_node(enabled);
	}

	if (begin_tree_node("Buffers", stereo_buffer, false)) {
		provider::align("\a");
		add_member_control(this, "Alpha Buffer", alpha_buffer, "toggle");
		add_member_control(this, "Double Buffer", context_config::double_buffer, "toggle");

		add_view("Depth Bits", depth_bits, "", "w=32", " ");
		add_member_control(this, "Depth Buffer", depth_buffer, "toggle", "w=160");
		
		add_view("Stencil Bits", stencil_bits, "", "w=32", " ");
		add_member_control(this, "Stencil Buffer", stencil_buffer, "toggle", "w=160");

		add_view("Accumulation Bits", accumulation_bits, "", "w=32", " ");
		add_member_control(this, "Accumulation Buffer", accumulation_buffer, "toggle", "w=160");

		add_view("Num Multi Samples", nr_multi_samples, "", "w=32", " ");
		add_member_control(this, "Multi Sample Buffer", multi_sample_buffer, "toggle", "w=160");

		add_member_control(this, "Stereo Buffer", stereo_buffer, "toggle");
		provider::align("\b");
		end_tree_node(stereo_buffer);
	}
	if (begin_tree_node("OpenGL", version, false)) {
		provider::align("\a");
		add_member_control(this, "Version", reinterpret_cast<cgv::type::DummyEnum&>(version), "dropdown", 
			"w=120;enums='detect=-1,1.0=10,1.1=11,1.2=12,1.3=13,1.4=14,1.5=15,2.0=20,2.1=21,3.0=30,3.1=31,3.2=32,3.3=33,4.0=40,4.1=41,4.2=42,4.3=43,4.4=44,4.5=45,4.6=46'", " ");
		add_view("", version_major, "", "w=32", " ");
		add_view(".", version_minor, "", "w=32");
		add_member_control(this, "Core", core_profile, "toggle", "w=60", " ");
		add_member_control(this, "Fwd Comp", forward_compatible, "toggle", "w=60", " ");
		add_member_control(this, "Debug", debug, "toggle", "w=60");
		configure_opengl_controls();
		provider::align("\b");
		end_tree_node(version);
	}
	if(begin_tree_node("Clear", bg_color_stack.top()[0], false)) {
		provider::align("\a");
		add_member_control(this, "Color", reinterpret_cast<cgv::media::color<float>&>(bg_color_stack.top()[0]));
		add_member_control(this, "Alpha", bg_color_stack.top()[3], "value_slider", "min=0;max=1;ticks=true;step=0.001");
		add_member_control(this, "Accum Color", reinterpret_cast<cgv::media::color<float>&>(bg_accum_color_stack.top()[0]));
		add_member_control(this, "Accum Alpha", bg_accum_color_stack.top()[3], "value_slider", "min=0;max=1;ticks=true;step=0.001");
		add_member_control(this, "Stencil", bg_stencil_stack.top(), "value_slider", "min=0;max=255;ticks=true;step=1");
		add_member_control(this, "Depth", bg_depth_stack.top(), "value_slider", "min=0;max=1;ticks=true;step=0.001");
		provider::align("\b");
		end_tree_node(bg_color_stack.top()[0]);
	}
	if (begin_tree_node("Compatibility", support_compatibility_mode, false)) {
		provider::align("\a");
		add_member_control(this, "Auto Set View in Current Shader Program", auto_set_view_in_current_shader_program, "check");
		add_member_control(this, "Auto Set Lights in Current Shader Program", auto_set_lights_in_current_shader_program, "check");
		add_member_control(this, "Auto Set Material in Current Shader Program", auto_set_material_in_current_shader_program, "check");
		add_member_control(this, "Support Compatibility Mode", support_compatibility_mode, "check");
		add_member_control(this, "Draw in Compatibility Mode", draw_in_compatibility_mode, "check");
		provider::align("\b");
		end_tree_node(support_compatibility_mode);
	}
	if (begin_tree_node("Defaults", current_material_is_textured, false)) {
		provider::align("\a");
		if (begin_tree_node("Default Render Flags", default_render_flags, false)) {
			add_gui("default_render_flags", default_render_flags, "bit_field_control",
			"enums='RPF_SET_PROJECTION = 1,RPF_SET_MODELVIEW = 2,RPF_SET_LIGHTS = 4,RPF_SET_MATERIAL = 8,\
RPF_SET_LIGHTS_ON=16,RPF_ENABLE_MATERIAL=32,RPF_CLEAR_COLOR=64,RPF_CLEAR_DEPTH=128,\
RPF_CLEAR_STENCIL=256,RPF_CLEAR_ACCUM=512,\
RPF_DRAWABLES_INIT_FRAME=1024,RPF_SET_STATE_FLAGS=2048,\
RPF_SET_CLEAR_COLOR=4096,RPF_SET_CLEAR_DEPTH=8192,RPF_SET_CLEAR_STENCIL=16384,RPF_SET_CLEAR_ACCUM=32768,\
RPF_DRAWABLES_DRAW=65536,RPF_DRAWABLES_FINISH_FRAME=131072,\
RPF_DRAW_TEXTUAL_INFO=262144,RPF_DRAWABLES_AFTER_FINISH=524288,RPF_HANDLE_SCREEN_SHOT=1048576");
			end_tree_node(default_render_flags);
		}


		if (begin_tree_node("Material", default_material, false)) {
			provider::align("\a");
			add_gui("material", default_material);
			provider::align("\b");
			end_tree_node(default_material);
		}
		for (int i = 0; i < nr_default_light_sources; ++i) {
			if (begin_tree_node("Light " + cgv::utils::to_string(i), default_light_source[i], false)) {
				provider::align("\a");
				add_gui("light", default_light_source[i]);
				provider::align("\b");
				end_tree_node(default_light_source[i]);
			}
		}
		provider::align("\b");
		end_tree_node(current_material_is_textured);
	}
}
