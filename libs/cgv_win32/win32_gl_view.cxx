#include <cgv/type/variant.h>
#include <cgv/base/action.h>
#include <cgv/base/traverser.h>
#include <cgv/render/drawable.h>
#include <cgv/gui/trigger.h>
#include <cgv/media/text/tokenizer.h>
#include <cgv/gui/event_handler.h>
#include "win32_gl_view.h"
#include <windows.h>
#include <windowsx.h>


using namespace cgv::gui;
using namespace cgv::render;
using namespace cgv::base;
using namespace cgv::media::text;

struct data
{
	HWND hWnd;
	WNDPROC lpOldProc;
	HDC hDC;
	HGLRC hRC;
	data() : hWnd(NULL), lpOldProc(NULL), hDC(NULL), hRC(NULL) {}
};

data& ref_data(void* data_ptr)
{
	return *((data*) data_ptr);
}

static LRESULT CALLBACK PluginWinProc(HWND, UINT, WPARAM, LPARAM);

namespace cgv {
	namespace gui {

void win32_gl_view::process_text_1(const std::string& text)
{
	process_text(text);
}


/// construct application
win32_gl_view::win32_gl_view(const std::string& name) 
	: group(name), on_new_message("*|"), on_unhandled_message("*|")
{
	data_ptr = new ::data();

	recreate_context = false;
	no_more_context = false;

	in_draw_method = false;
	redraw_request = false;

	last_mouse_x = last_mouse_y = 32683;

	stereo_support = false;

	connect(out_stream.write, this, &win32_gl_view::process_text_1);
}


///
cgv::base::group* win32_gl_view::get_group_interface()
{
	return this;
}

///
void win32_gl_view::self_reflect(cgv::base::self_reflection_handler& srh)
{
	srh.reflect_member("show_help", show_help) &&
	srh.reflect_member("show_stats", show_stats) &&
	srh.reflect_member("font_size", info_font_size) &&
	srh.reflect_member("tab_size", tab_size) &&
	srh.reflect_member("bg_r", bg_r) &&
	srh.reflect_member("bg_g", bg_g) &&
	srh.reflect_member("bg_b", bg_b) &&
	srh.reflect_member("bg_a", bg_a) &&
	srh.reflect_member("bg_index", current_background);
}

void win32_gl_view::on_set(void* member_ptr)
{
	if (member_ptr == &current_background)
		set_bg_clr_idx(current_background);
	post_redraw();
}

/// returns the property declaration
std::string win32_gl_view::get_property_declarations()
{
	return cgv::base::group::get_property_declarations()+";quad_buffer:bool";
}

/// abstract interface for the setter 
bool win32_gl_view::set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
{
	if (property == "quad_buffer") {
		bool new_value;
		cgv::type::get_variant(new_value, value_type, value_ptr);
		if (new_value != stereo_support) {
			recreate_context = true;
			destroy_context();
			stereo_support = new_value;
			construct_context();
			recreate_context = false;
			post_redraw();
		}
		return true;
	}
	return false;
}

/// abstract interface for the getter 
bool win32_gl_view::get_void(const std::string& property, const std::string& value_type, void* value_ptr)
{
	if (property == "quad_buffer") {
		cgv::type::set_variant(stereo_support, value_type, value_ptr);
		return true;
	}
	return false;
}

///
void* win32_gl_view::get_data()
{
	return data_ptr;
}

void win32_gl_view::attach(void* window_handle)
{
	::data& d(ref_data(data_ptr));

	d.hWnd = (HWND)window_handle;
	if(d.hWnd == NULL)
		return;

	// do our drawing to it
	d.lpOldProc = SubclassWindow(d.hWnd, (WNDPROC)PluginWinProc);
	SetWindowLong(d.hWnd, GWL_USERDATA, (LONG)this);
	construct_context();
}

void win32_gl_view::detach()
{
	::data& d(ref_data(data_ptr));
	wglMakeCurrent(NULL,NULL);
	wglDeleteContext(d.hRC);
	SubclassWindow(d.hWnd, d.lpOldProc);
	delete &d;
	data_ptr = 0;
}

void win32_gl_view::construct_context()
{
	::data& d(ref_data(data_ptr));

	PIXELFORMATDESCRIPTOR pfd;
	int format;
	
	// get the device context (DC)
	d.hDC = GetDC(d.hWnd);
	
	// set the pixel format for the DC
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	if (stereo_support)
		pfd.dwFlags |= PFD_STEREO;

	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	format = ChoosePixelFormat( d.hDC, &pfd );
	SetPixelFormat( d.hDC, format, &pfd );
	
	// create and enable the render context (RC)
	d.hRC = wglCreateContext(d.hDC);
	wglMakeCurrent(d.hDC,d.hRC);
}

void win32_gl_view::destroy_context()
{
	::data& d(ref_data(data_ptr));

	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( d.hRC );
	ReleaseDC( d.hWnd, d.hDC );
}

/// helper method to remove a child
void win32_gl_view::clear_child(cgv::base::base_ptr child)
{
	cgv::base::single_method_action<cgv::render::drawable,void,cgv::render::context&> sma(*this, &cgv::render::drawable::clear, false, false);
	cgv::base::traverser(sma,"nc").traverse(child);

	on_remove_child(child);
}

/// append child and return index of appended child
unsigned int win32_gl_view::append_child(cgv::base::base_ptr child)
{
	if (child->get_interface<cgv::render::drawable>() == 0)
		return -1;
	unsigned int i = cgv::base::group::append_child(child);
	configure_new_child(child);
	return i;
}
/// remove all elements of the vector that point to child, return the number of removed children
unsigned int win32_gl_view::remove_child(cgv::base::base_ptr child)
{
	clear_child(child);
	return group::remove_child(child);
}
/// remove all children
void win32_gl_view::remove_all_children()
{
	for (unsigned int i=0; i<get_nr_children(); ++i)
		clear_child(get_child(i));
	group::remove_all_children();
}
/// insert a child at the given position
void win32_gl_view::insert_child(unsigned int i, cgv::base::base_ptr child)
{
	if (child->get_interface<cgv::render::drawable>() == 0)
		return;
	cgv::base::group::insert_child(i,child);
	configure_new_child(child);
}


void win32_gl_view::register_object(cgv::base::base_ptr object, const std::string& options)
{
	if (object->get_interface<factory>()) {
		factories.push_back(object);
		unsigned shortcut = 0;
		if (!options.empty()) {
			std::vector<token> toks;
			tokenizer(options).set_ws(";").set_skip("'\"","'\"").bite_all(toks);
			for (unsigned i=0; i<toks.size(); ++i) {
				std::vector<token> sub_toks;
				tokenizer(toks[i]).set_ws("=").set_skip("'\"","'\"").bite_all(sub_toks);
				if (sub_toks.size() == 2) {
					if (sub_toks[0] == "shortcut") {
						if (sub_toks[1].size() > 0 && (sub_toks[1][0] == '"' || sub_toks[1][0] == '\'')) {
							++sub_toks[1].begin;
						}
						shortcut = sub_toks[1][0];
					}
				}
			}
		}
		shortcuts.push_back(shortcut);
	}
	else if (object->get_interface<drawable>())
		append_child(object);
}

void win32_gl_view::unregister_object(cgv::base::base_ptr object, const std::string& options)
{
	if (object->get_interface<factory>()) {
		for (unsigned i=0; i<factories.size(); ++i)
			if (factories[i] == object) {
				factories.erase(factories.begin()+i);
				shortcuts.erase(shortcuts.begin()+i);
				--i;
			}
	}
	else if (object->get_interface<drawable>())
		remove_child(object);
}

/// return whether the context is currently in process of rendering
bool win32_gl_view::in_render_process() const
{
	return in_draw_method;
}

/// return whether the context is current
bool win32_gl_view::is_current() const
{
	return false;
}

/// make the current context current
bool win32_gl_view::make_current() const
{
	::data& d(ref_data(data_ptr));
	wglMakeCurrent(d.hDC,d.hRC);
	return true;
}

/// return the width of the window
unsigned int win32_gl_view::get_width() const
{
	::data& d(ref_data(data_ptr));
    RECT r;
	GetWindowRect(d.hWnd, &r);
	return r.right - r.left;
}

/// return the height of the window
unsigned int win32_gl_view::get_height() const
{
	::data& d(ref_data(data_ptr));
    RECT r;
	GetWindowRect(d.hWnd, &r);
	return r.bottom - r.top;
}

/// resize the context to the given dimensions
void win32_gl_view::resize(unsigned int width, unsigned int height)
{
	::data& d(ref_data(data_ptr));
    RECT r;
	GetWindowRect(d.hWnd, &r);
	MoveWindow(d.hWnd, r.left, r.top, width, height, TRUE);
}

/// the context will be redrawn when the system is idle again
void win32_gl_view::post_redraw()
{
	::data& d(ref_data(data_ptr));
	PostMessage(d.hWnd, WM_PAINT, NULL, NULL);
}

/// the context will be redrawn right now. This method cannot be called inside the following methods of a drawable: init, init_frame, draw, finish_draw
void win32_gl_view::force_redraw()
{
	::data& d(ref_data(data_ptr));
	PostMessage(d.hWnd, WM_PAINT, NULL, NULL);
}

/// enable the given font face with the given size in pixels
void win32_gl_view::enable_font_face(cgv::media::font::font_face_ptr font_face, float font_size)
{
}

Keys translate_vk_code(unsigned vk_code)
{
	static Keys from_8[] = {
	/* 08 */ KEY_Back_Space, KEY_Tab,       (Keys)0,      (Keys)0,   KEY_Delete,    KEY_Enter, (Keys)0,     (Keys)0, 
	/* 10 */ KEY_Left_Shift, KEY_Left_Ctrl, KEY_Left_Alt, KEY_Pause, KEY_Caps_Lock, (Keys)0,   (Keys)0,     (Keys)0, 
	/* 18 */ (Keys)0,        (Keys)0,       (Keys)0,      KEY_Escape,(Keys)0,       (Keys)0,   (Keys)0,     (Keys)0, 
	/* 20 */ KEY_Space,      KEY_Page_Up,   KEY_Page_Down,KEY_End,   KEY_Home,      KEY_Left,   KEY_Up,     KEY_Right, 
	/* 28 */ KEY_Down,       (Keys)0,       KEY_Print,    (Keys)0,   (Keys)0,       KEY_Insert, KEY_Delete, KEY_F1 };

	static Keys from_60[] = { 
	/* 60 */ KEY_Num_0, KEY_Num_1, KEY_Num_2,   KEY_Num_3,   KEY_Num_4,     KEY_Num_5,   KEY_Num_6,   KEY_Num_7,
	/* 68 */ KEY_Num_8, KEY_Num_9, KEY_Num_Mul, KEY_Num_Add, KEY_Num_Enter, KEY_Num_Sub, KEY_Num_Dot, KEY_Num_Div,
	/* 70 */ KEY_F1,    KEY_F2,    KEY_F3,      KEY_F4,      KEY_F5,        KEY_F6,      KEY_F7,      KEY_F8,
	/* 78 */ KEY_F9,    KEY_F10,   KEY_F11,     KEY_F12 };

	if (vk_code >= 8 && vk_code < 48)
		return from_8[vk_code - 8];
	if (vk_code >= 96 && vk_code < 124)
		return from_60[vk_code - 96];
	return (Keys) vk_code;
}

bool win32_gl_view::handle(cgv::gui::event& e)
{
	if (e.get_kind() != EID_KEY)
		return false;
	key_event& ke = static_cast<key_event&>(e);
	if (ke.get_action() == KA_PRESS) {
		switch (ke.get_key()) {
		case KEY_Delete :
			if (ke.get_modifiers() == 0) {
				if (get_focused_child() != -1) {
					cgv::base::unregister_object(get_child(get_focused_child()), "");
//						remove_child(get_child(get_focused_child()));
					if (get_focused_child() >= (int)get_nr_children())
						traverse_policy::set_focused_child(get_focused_child()-1);
					post_redraw();
					return true;
				}
			}
			break;
		case KEY_Tab :
			if (ke.get_modifiers() == EM_SHIFT) {
				traverse_policy::set_focused_child(get_focused_child()-1);
				if (get_focused_child() == -2)
					traverse_policy::set_focused_child((int)get_nr_children()-1);
				if (show_stats)
					post_redraw();
				return true;
			}
			else if (ke.get_modifiers() == 0) {
				traverse_policy::set_focused_child(get_focused_child()+1);
				if (get_focused_child() >= (int)get_nr_children())
					traverse_policy::set_focused_child(-1);
				if (show_stats)
					post_redraw();
				return true;
			}
			break;
		}
		if (ke.get_modifiers() == EM_CTRL) {
			for (unsigned i=0; i<factories.size(); ++i) {
				if (ke.get_key() == shortcuts[i]) {
					cgv::base::register_object(factories[i]->get_interface<factory>()->create_object());
					post_redraw();
					return true;
				}
			}
		}
	}
	return false;
}

void win32_gl_view::stream_help(std::ostream& os)
{
}

bool win32_gl_view::dispatch_key_event(cgv::gui::KeyAction ka, cgv::gui::Keys k)
{
	unsigned char modifiers = 0;
	if (GetKeyState(VK_CONTROL) < 0)
		modifiers += (unsigned char)EM_CTRL;
	if (GetKeyState(VK_SHIFT) < 0)
		modifiers += (unsigned char)EM_SHIFT;
	if (GetKeyState(VK_MENU) < 0) 
		modifiers += (unsigned char)EM_ALT;

	double time = 0;
	if (get_trigger_server())
		time = get_trigger_server()->get_current_time();
 
	key_event ke((unsigned short)k, ka, (unsigned char)k, modifiers, 0, time);

	if (handle(ke))
		return true;

	ke.stream_out(std::cout);
	single_method_action<event_handler,bool,event&> sma(ke,&event_handler::handle);
	if (traverser(sma).traverse(group_ptr(this)))
		return true;
	return false;
}

bool win32_gl_view::dispatch_mouse_event(short x, short y, cgv::gui::MouseAction ma, int win32_state, 
												 unsigned char button, short dx, short dy)
{
	if (dx == 32683)
		if (last_mouse_x == 32683)
			dx = 0;
		else
			dx = x-last_mouse_x;
	last_mouse_x = x;
	if (dy == 32683)
		if (last_mouse_y == 32683)
			dy = 0;
		else
			dy = y-last_mouse_y;
	last_mouse_y = y;
	unsigned char button_state = 0;
	if (win32_state & MK_LBUTTON)
		button_state += (unsigned char)MB_LEFT_BUTTON;
	if (win32_state & MK_MBUTTON)
		button_state += (unsigned char)MB_MIDDLE_BUTTON;
	if (win32_state & MK_RBUTTON)
		button_state += (unsigned char)MB_RIGHT_BUTTON;

	unsigned char modifiers = 0;
	if (win32_state & MK_CONTROL)
		modifiers += (unsigned char)EM_CTRL;
	if (win32_state & MK_SHIFT)
		modifiers += (unsigned char)EM_SHIFT;
	if (GetKeyState(VK_LMENU) > 1 || GetKeyState(VK_RMENU) > 1) 
		modifiers += (unsigned char)EM_ALT;

	double time = 0;
	if (get_trigger_server())
		time = get_trigger_server()->get_current_time();

	cgv::gui::mouse_event me(x,y, ma, button_state, button, dx, dy, modifiers, 0, time);

	single_method_action<event_handler,bool,event&> sma(me,&event_handler::handle);
	if (traverser(sma).traverse(group_ptr(this)))
		return true;
	return false;
}

	}
}


static LRESULT CALLBACK PluginWinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	cgv::gui::win32_gl_view *swi= (cgv::gui::win32_gl_view*) GetWindowLong(hWnd, GWL_USERDATA);
	data& d(ref_data(swi->get_data()));
	if (swi->on_new_message(message,wParam,lParam))
		return 0;
	switch (message) {
	case WM_MOUSEWHEEL :
		if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_WHEEL, 
			GET_KEYSTATE_WPARAM(wParam), 0, 32683, -GET_WHEEL_DELTA_WPARAM(wParam)/WHEEL_DELTA))
				return 0;
		break;
	case WM_LBUTTONDOWN :
		if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_PRESS, 
			GET_KEYSTATE_WPARAM(wParam), MB_LEFT_BUTTON))
				return 0;
		break;
	case WM_LBUTTONUP :
		if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_RELEASE, 
			GET_KEYSTATE_WPARAM(wParam), MB_LEFT_BUTTON))
				return 0;
		break;
	case WM_MBUTTONDOWN :
		if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_PRESS, 
			GET_KEYSTATE_WPARAM(wParam), MB_MIDDLE_BUTTON))
				return 0;
		break;
	case WM_MBUTTONUP :
		if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_RELEASE, 
			GET_KEYSTATE_WPARAM(wParam), MB_MIDDLE_BUTTON))
				return 0;
		break;
	case WM_RBUTTONDOWN :
		if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_PRESS, 
			GET_KEYSTATE_WPARAM(wParam), MB_RIGHT_BUTTON))
				return 0;
		break;
	case WM_RBUTTONUP :
		if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_RELEASE, 
			GET_KEYSTATE_WPARAM(wParam), MB_RIGHT_BUTTON))
				return 0;
		break;
	case WM_MOUSEMOVE :
		if (GET_KEYSTATE_WPARAM(wParam) & (MK_LBUTTON|MK_MBUTTON|MK_RBUTTON)) {
			if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_DRAG, 
				GET_KEYSTATE_WPARAM(wParam), MB_NO_BUTTON))
					return 0;
		}
		else {
			if (swi->dispatch_mouse_event(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam),MA_MOVE, 
				GET_KEYSTATE_WPARAM(wParam), MB_NO_BUTTON))
					return 0;
		}
		break;
	case WM_KEYDOWN :
		if (swi->dispatch_key_event(KA_PRESS, cgv::gui::translate_vk_code(wParam)))
				return 0;
		break;
	case WM_KEYUP :
		if (swi->dispatch_key_event(KA_RELEASE, cgv::gui::translate_vk_code(wParam)))
				return 0;
		break;
	case WM_PAINT:
		swi->make_current();
		swi->render_pass(cgv::render::RP_MAIN, swi->get_default_render_pass_flags());
		glFlush();
		SwapBuffers(d.hDC);
		return 0;
	case WM_CLOSE:
		swi->destroy_context();
		PostQuitMessage( 0 );
		return 0;
	default:
		break;				
	}
	if (swi->on_unhandled_message(message,wParam,lParam))
		return 0;
	return DefWindowProc( hWnd, message, wParam, lParam );
}
