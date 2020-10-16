#include "glfw_imgui_driver.h"

//#include "imgui.h"

#include <cgv/gui/base_provider_generator.h>
#include <cgv/gui/menu_provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/utils/scan.h>
#include <cgv/base/traverser.h>
#include <cgv/type/variant.h>
#include <cgv_gl/gl/gl_context.h>
#include <cgv_gl/gl/gl_performance_monitor.h>
#include <cgv/render/context.h>
#include <glfw/glfw3.h>

///
glfw_imgui_driver::glfw_imgui_driver()
{

}
/**@name application management*/
//@{
/// fill list of monitor descriptions
bool glfw_imgui_driver::enumerate_monitors(std::vector<monitor_description>& monitor_descriptions)
{
	int n;
	GLFWmonitor** monitors = glfwGetMonitors(&n);
	for (int i = 0; i < n; ++i) {
		monitor_description md;
		glfwGetMonitorWorkarea(monitors[i], &md.x, &md.y, reinterpret_cast<int*>(&md.w), reinterpret_cast<int*>(&md.h));
		int w_mm, h_mm;
		glfwGetMonitorPhysicalSize(monitors[i], &w_mm, &h_mm);
		md.dpi_x = (25.4f * md.w / w_mm);
		md.dpi_y = (25.4f * md.h / h_mm);
		monitor_descriptions.push_back(md);
	}
	return true;
}

struct glfw_base
{
	GLFWcursor* cursor;
	std::string tooltip;
	std::string alignment;
	int32_t color, text_color, label_color;
	int w, h;
	std::string get_property_declarations() const
	{
		return  "x:int32;y:int32;w:int32;h:int32;"
			"label:string;name:string;tooltip:string;"
			"active:bool;show:bool;color:int32;text_color:int32;label_color:int32;align:string;cursor:string";
	}

};

class glfw_generic_window : public cgv::gui::window, public glfw_base
{
protected:
	GLFWwindow* glfw_window;
	std::string title;
	std::string menu_order;
	std::string dock_order;
	bool menu;
	bool gui;
	bool stats;

	bool dispatch_event(cgv::gui::event& e)
	{
		cgv::base::single_method_action<cgv::gui::event_handler, bool, event&> sma((event&)e, &cgv::gui::event_handler::handle);
		return cgv::base::traverser(sma).traverse(cgv::base::group_ptr(this));
	}
	unsigned short translate_key(int key)
	{
		switch (key) {
		case GLFW_KEY_SPACE              : return cgv::gui::KEY_Space;
		case GLFW_KEY_APOSTROPHE         : return '´';
		case GLFW_KEY_COMMA              : return ',';
		case GLFW_KEY_MINUS              : return '-';
		case GLFW_KEY_PERIOD             : return '.';
		case GLFW_KEY_SLASH              : return '/';
		case GLFW_KEY_0                  : return '0';
		case GLFW_KEY_1                  : return '1';
		case GLFW_KEY_2                  : return '2';
		case GLFW_KEY_3                  : return '3';
		case GLFW_KEY_4                  : return '4';
		case GLFW_KEY_5                  : return '5';
		case GLFW_KEY_6                  : return '6';
		case GLFW_KEY_7                  : return '7';
		case GLFW_KEY_8                  : return '8';
		case GLFW_KEY_9                  : return '9';
		case GLFW_KEY_SEMICOLON          : return ';';
		case GLFW_KEY_EQUAL              : return '=';
		case GLFW_KEY_A                  : return 'A';
		case GLFW_KEY_B                  : return 'B';
		case GLFW_KEY_C                  : return 'C';
		case GLFW_KEY_D                  : return 'D';
		case GLFW_KEY_E                  : return 'E';
		case GLFW_KEY_F                  : return 'F';
		case GLFW_KEY_G                  : return 'G';
		case GLFW_KEY_H                  : return 'H';
		case GLFW_KEY_I                  : return 'I';
		case GLFW_KEY_J                  : return 'J';
		case GLFW_KEY_K                  : return 'K';
		case GLFW_KEY_L                  : return 'L';
		case GLFW_KEY_M                  : return 'M';
		case GLFW_KEY_N                  : return 'N';
		case GLFW_KEY_O                  : return 'O';
		case GLFW_KEY_P                  : return 'P';
		case GLFW_KEY_Q                  : return 'Q';
		case GLFW_KEY_R                  : return 'R';
		case GLFW_KEY_S                  : return 'S';
		case GLFW_KEY_T                  : return 'T';
		case GLFW_KEY_U                  : return 'U';
		case GLFW_KEY_V                  : return 'V';
		case GLFW_KEY_W                  : return 'W';
		case GLFW_KEY_X                  : return 'X';
		case GLFW_KEY_Y                  : return 'Y';
		case GLFW_KEY_Z                  : return 'Z';
		case GLFW_KEY_LEFT_BRACKET       : return '[';
		case GLFW_KEY_BACKSLASH          : return '\\';
		case GLFW_KEY_RIGHT_BRACKET      : return ']';
		case GLFW_KEY_GRAVE_ACCENT       : return '`';
		case GLFW_KEY_WORLD_1            : return 0;
		case GLFW_KEY_WORLD_2            : return 0;
		case GLFW_KEY_ESCAPE             : return cgv::gui::KEY_Escape;
		case GLFW_KEY_ENTER              : return cgv::gui::KEY_Enter;
		case GLFW_KEY_TAB                : return cgv::gui::KEY_Tab;
		case GLFW_KEY_BACKSPACE          : return cgv::gui::KEY_Back_Space;
		case GLFW_KEY_INSERT             : return cgv::gui::KEY_Insert;
		case GLFW_KEY_DELETE             : return cgv::gui::KEY_Delete;
		case GLFW_KEY_RIGHT              : return cgv::gui::KEY_Right;
		case GLFW_KEY_LEFT               : return cgv::gui::KEY_Left;
		case GLFW_KEY_DOWN               : return cgv::gui::KEY_Down;
		case GLFW_KEY_UP                 : return cgv::gui::KEY_Up;
		case GLFW_KEY_PAGE_UP            : return cgv::gui::KEY_Page_Up;
		case GLFW_KEY_PAGE_DOWN          : return cgv::gui::KEY_Page_Down;
		case GLFW_KEY_HOME               : return cgv::gui::KEY_Home;
		case GLFW_KEY_END                : return cgv::gui::KEY_End;
		case GLFW_KEY_CAPS_LOCK          : return cgv::gui::KEY_Caps_Lock;
		case GLFW_KEY_SCROLL_LOCK        : return cgv::gui::KEY_Scroll_Lock;
		case GLFW_KEY_NUM_LOCK           : return cgv::gui::KEY_Num_Lock;
		case GLFW_KEY_PRINT_SCREEN       : return cgv::gui::KEY_Print;
		case GLFW_KEY_PAUSE              : return cgv::gui::KEY_Pause;
		case GLFW_KEY_F1                 : return cgv::gui::KEY_F1;
		case GLFW_KEY_F2                 : return cgv::gui::KEY_F2;
		case GLFW_KEY_F3                 : return cgv::gui::KEY_F3;
		case GLFW_KEY_F4                 : return cgv::gui::KEY_F4;
		case GLFW_KEY_F5                 : return cgv::gui::KEY_F5;
		case GLFW_KEY_F6                 : return cgv::gui::KEY_F6;
		case GLFW_KEY_F7                 : return cgv::gui::KEY_F7;
		case GLFW_KEY_F8                 : return cgv::gui::KEY_F8;
		case GLFW_KEY_F9                 : return cgv::gui::KEY_F9;
		case GLFW_KEY_F10                : return cgv::gui::KEY_F10;
		case GLFW_KEY_F11                : return cgv::gui::KEY_F11;
		case GLFW_KEY_F12                : return cgv::gui::KEY_F12;
		case GLFW_KEY_F13                : return cgv::gui::KEY_F13;
		case GLFW_KEY_F14                : return cgv::gui::KEY_F14;
		case GLFW_KEY_F15                : return cgv::gui::KEY_F15;
		case GLFW_KEY_F16                : return cgv::gui::KEY_F16;
		case GLFW_KEY_F17                : return cgv::gui::KEY_F17;
		case GLFW_KEY_F18                : return cgv::gui::KEY_F18;
		case GLFW_KEY_F19                : return cgv::gui::KEY_F19;
		case GLFW_KEY_F20                : return cgv::gui::KEY_F20;
		case GLFW_KEY_F21                : return cgv::gui::KEY_F21;
		case GLFW_KEY_F22                : return cgv::gui::KEY_F22;
		case GLFW_KEY_F23                : return cgv::gui::KEY_F23;
		case GLFW_KEY_F24                : return cgv::gui::KEY_F24;
		case GLFW_KEY_F25                : return cgv::gui::KEY_F25;
		case GLFW_KEY_KP_0               : return cgv::gui::KEY_Num_0;
		case GLFW_KEY_KP_1               : return cgv::gui::KEY_Num_1;
		case GLFW_KEY_KP_2               : return cgv::gui::KEY_Num_2;
		case GLFW_KEY_KP_3               : return cgv::gui::KEY_Num_3;
		case GLFW_KEY_KP_4               : return cgv::gui::KEY_Num_4;
		case GLFW_KEY_KP_5               : return cgv::gui::KEY_Num_5;
		case GLFW_KEY_KP_6               : return cgv::gui::KEY_Num_6;
		case GLFW_KEY_KP_7               : return cgv::gui::KEY_Num_7;
		case GLFW_KEY_KP_8               : return cgv::gui::KEY_Num_8;
		case GLFW_KEY_KP_9               : return cgv::gui::KEY_Num_9;
		case GLFW_KEY_KP_DECIMAL         : return cgv::gui::KEY_Num_Dot;
		case GLFW_KEY_KP_DIVIDE          : return cgv::gui::KEY_Num_Div;
		case GLFW_KEY_KP_MULTIPLY        : return cgv::gui::KEY_Num_Mul;
		case GLFW_KEY_KP_SUBTRACT        : return cgv::gui::KEY_Num_Sub;
		case GLFW_KEY_KP_ADD             : return cgv::gui::KEY_Num_Add;
		case GLFW_KEY_KP_ENTER           : return cgv::gui::KEY_Num_Enter;
		case GLFW_KEY_KP_EQUAL           : return cgv::gui::KEY_Last;
		case GLFW_KEY_LEFT_SHIFT         : return cgv::gui::KEY_Left_Shift;
		case GLFW_KEY_LEFT_CONTROL       : return cgv::gui::KEY_Left_Ctrl;
		case GLFW_KEY_LEFT_ALT           : return cgv::gui::KEY_Left_Alt;
		case GLFW_KEY_LEFT_SUPER         : return cgv::gui::KEY_Left_Meta;
		case GLFW_KEY_RIGHT_SHIFT        : return cgv::gui::KEY_Right_Shift;
		case GLFW_KEY_RIGHT_CONTROL      : return cgv::gui::KEY_Right_Ctrl;
		case GLFW_KEY_RIGHT_ALT          : return cgv::gui::KEY_Right_Alt;
		case GLFW_KEY_RIGHT_SUPER        : return cgv::gui::KEY_Right_Meta;
		case GLFW_KEY_MENU               : return cgv::gui::KEY_Menu;
		default: return 0;
		}
	}
	cgv::gui::KeyAction translate_key_action(int action)
	{
		switch (action) {
		case GLFW_PRESS: return cgv::gui::KA_PRESS;
		case GLFW_RELEASE: return cgv::gui::KA_RELEASE;
		case GLFW_REPEAT: return cgv::gui::KA_REPEAT;
		default: return cgv::gui::KA_PRESS;
		}
	}
	unsigned char translate_modifiers(int mods)
	{
		unsigned char m = 0;
		if ((mods & GLFW_MOD_SHIFT) != 0)
			m += cgv::gui::EM_SHIFT;
		if ((mods & GLFW_MOD_ALT) != 0)
			m += cgv::gui::EM_ALT;
		if ((mods & GLFW_MOD_CONTROL) != 0)
			m += cgv::gui::EM_CTRL;
		if ((mods & GLFW_MOD_SUPER) != 0)
			m += cgv::gui::EM_META;
		return m;
	}
	unsigned char translate_toggle_keys(int mods)
	{
		unsigned char m = 0;
		if ((mods & GLFW_MOD_CAPS_LOCK) != 0)
			m += cgv::gui::ETK_CAPS_LOCK;
		if ((mods & GLFW_MOD_NUM_LOCK) != 0)
			m += cgv::gui::ETK_NUM_LOCK;
		return m;
	}
	void key(int key, int scancode, int action, int mods)
	{
		unsigned short _key = translate_key(key);
		cgv::gui::KeyAction _action = translate_key_action(action);
		unsigned char _char = scancode;
		unsigned char _modifiers = translate_modifiers(mods);
		unsigned char _toggle_keys = translate_toggle_keys(mods);
		double _time = glfwGetTime();
		cgv::gui::key_event ke(_key, _action, _char, _modifiers, _toggle_keys, _time);
		dispatch_event(ke);
	}
	unsigned char query_modifiers() const
	{
		unsigned char m = 0;
		if (glfwGetKey(glfw_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey(glfw_window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
			m += cgv::gui::EM_SHIFT;
		if (glfwGetKey(glfw_window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
			glfwGetKey(glfw_window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS)
			m += cgv::gui::EM_ALT;
		if (glfwGetKey(glfw_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey(glfw_window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
			m += cgv::gui::EM_CTRL;
		if (glfwGetKey(glfw_window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ||
			glfwGetKey(glfw_window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS)
			m += cgv::gui::EM_META;
		return m;
	}
	unsigned char query_toggle_keys() const
	{
		unsigned char m = 0;
		if (glfwGetKey(glfw_window, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS)
			m += cgv::gui::ETK_CAPS_LOCK;
		if (glfwGetKey(glfw_window, GLFW_KEY_NUM_LOCK) == GLFW_PRESS)
			m += cgv::gui::ETK_NUM_LOCK;
		if (glfwGetKey(glfw_window, GLFW_KEY_SCROLL_LOCK) == GLFW_PRESS)
			m += cgv::gui::ETK_SCROLL_LOCK;
		return m;
	}
	unsigned char query_button_state() const
	{
		unsigned char m = 0;
		if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			m += cgv::gui::MB_LEFT_BUTTON;
		if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			m += cgv::gui::MB_RIGHT_BUTTON;
		if (glfwGetMouseButton(glfw_window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
			m += cgv::gui::MB_MIDDLE_BUTTON;
		return m;
	}
	/// translate cursor position event to mouse move event
	void cursor_position(double xpos, double ypos)
	{
		cgv::gui::mouse_event me((int)xpos, (int)ypos, cgv::gui::MA_MOVE, query_button_state(), 0, 0, 0, query_modifiers(), query_toggle_keys(), glfwGetTime());
		dispatch_event(me);
	}
	/// translate mouse button event to mouse press / release event
	void mouse_button(int button, int action, int mods)
	{
		double xpos, ypos;
		glfwGetCursorPos(glfw_window, &xpos, &ypos);
		cgv::gui::MouseButton mb;
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT: mb = cgv::gui::MB_LEFT_BUTTON; break;
		case GLFW_MOUSE_BUTTON_MIDDLE: mb = cgv::gui::MB_MIDDLE_BUTTON; break;
		case GLFW_MOUSE_BUTTON_RIGHT: mb = cgv::gui::MB_RIGHT_BUTTON; break;
		default: return;
		}
		cgv::gui::MouseAction ma = (action == GLFW_PRESS ? cgv::gui::MA_PRESS : cgv::gui::MA_RELEASE);
		cgv::gui::mouse_event me((int)xpos, (int)ypos, ma, query_button_state(), mb, 0, 0, query_modifiers(), query_toggle_keys(), glfwGetTime());
		dispatch_event(me);
	}
	/// translate window enter event
	void cursor_enter(int entered)
	{
		double xpos, ypos;
		glfwGetCursorPos(glfw_window, &xpos, &ypos);
		cgv::gui::MouseAction ma = (entered ? cgv::gui::MA_ENTER : cgv::gui::MA_LEAVE);
		cgv::gui::mouse_event me((int)xpos, (int)ypos, ma, query_button_state(), 0, 0, 0, query_modifiers(), query_toggle_keys(), glfwGetTime());
		dispatch_event(me);
	}
	/// translate window scroll event
	void scroll(double xoffset, double yoffset)
	{
		double xpos, ypos;
		glfwGetCursorPos(glfw_window, &xpos, &ypos);
		cgv::gui::mouse_event me((int)xpos, (int)ypos, cgv::gui::MA_WHEEL, query_button_state(), 0, (short)xoffset, (short)yoffset, query_modifiers(), query_toggle_keys(), glfwGetTime());
		dispatch_event(me);
	}
	virtual void on_resize(int w, int h) { }
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		static_cast<glfw_generic_window*>(glfwGetWindowUserPointer(window))->key(key, scancode, action, mods);
	}
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		static_cast<glfw_generic_window*>(glfwGetWindowUserPointer(window))->cursor_position(xpos, ypos);
	}
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		static_cast<glfw_generic_window*>(glfwGetWindowUserPointer(window))->mouse_button(button, action, mods);
	}
	static void cursor_enter_callback(GLFWwindow* window, int entered)
	{
		static_cast<glfw_generic_window*>(glfwGetWindowUserPointer(window))->cursor_enter(entered);
	}
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		static_cast<glfw_generic_window*>(glfwGetWindowUserPointer(window))->scroll(xoffset, yoffset);
	}
	static void resize_callback(GLFWwindow* window, int width, int height)
	{
		static_cast<glfw_generic_window*>(glfwGetWindowUserPointer(window))->on_resize(width, height);
	}
	static int get_cursor_index(const std::string& name)
	{
		int ei = cgv::utils::get_element_index(name, "default,arrow,cross,wait,insert,hand,help,move,ns,we,nwse,nesw,no,invisible", ',');
		if (ei == -1)
			ei = 0;
		return ei;
	}
	static const std::vector<GLFWcursor*>& query_standard_cursors()
	{
		static std::vector<GLFWcursor*> cursors;
		if (cursors.empty()) {
			cursors.push_back(glfwCreateStandardCursor(GLFW_ARROW_CURSOR)); // arrow is default cursor
			cursors.push_back(cursors.front());
			cursors.push_back(glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR));
			cursors.push_back(cursors.front()); // todo: wait cursor
			cursors.push_back(glfwCreateStandardCursor(GLFW_IBEAM_CURSOR));
			cursors.push_back(glfwCreateStandardCursor(GLFW_HAND_CURSOR));
			cursors.push_back(cursors.front()); // todo: help cursor
			cursors.push_back(cursors.front()); // todo: move cursor
			cursors.push_back(glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR));
			cursors.push_back(glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR));
			cursors.push_back(cursors.front()); // todo: nwse cursor
			cursors.push_back(cursors.front()); // todo: nesw cursor
			cursors.push_back(cursors.front()); // todo: no cursor
		}
		return cursors;
	}
	bool recreate_glfw_window(int W, int H, const std::string& title, const cgv::render::render_config* config_ptr = 0, glfw_generic_window* share = 0)
	{
		if (config_ptr) {
			// buffer settings
			glfwWindowHint(GLFW_STEREO, config_ptr->stereo_buffer ? GLFW_TRUE : GLFW_FALSE);
			glfwWindowHint(GLFW_DOUBLEBUFFER, config_ptr->double_buffer ? GLFW_TRUE : GLFW_FALSE);
			glfwWindowHint(GLFW_STENCIL_BITS, config_ptr->stencil_buffer ? (config_ptr->stencil_bits == -1 ? 8 : config_ptr->stencil_bits) : 0);
			glfwWindowHint(GLFW_SAMPLES, config_ptr->multi_sample_buffer ? (config_ptr->nr_multi_samples == -1 ? 4 : config_ptr->nr_multi_samples) : 0);
			glfwWindowHint(GLFW_DEPTH_BITS, config_ptr->depth_buffer ? (config_ptr->depth_bits == -1 ? 24 : config_ptr->depth_bits) : 0);
			if (config_ptr->accumulation_buffer) {
				glfwWindowHint(GLFW_ACCUM_RED_BITS, config_ptr->accumulation_bits == -1 ? 16 : config_ptr->accumulation_bits);
				glfwWindowHint(GLFW_ACCUM_GREEN_BITS, config_ptr->accumulation_bits == -1 ? 16 : config_ptr->accumulation_bits);
				glfwWindowHint(GLFW_ACCUM_BLUE_BITS, config_ptr->accumulation_bits == -1 ? 16 : config_ptr->accumulation_bits);
				glfwWindowHint(GLFW_ACCUM_RED_BITS, config_ptr->alpha_buffer ? (config_ptr->accumulation_bits == -1 ? 16 : config_ptr->accumulation_bits) : 0);
			}
			// opengl and context settings
			glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, config_ptr->version_major != -1 ? 1 : config_ptr->version_major);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, config_ptr->version_major != -1 ? 0 : config_ptr->version_minor);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, config_ptr->forward_compatible ? GLFW_TRUE : GLFW_FALSE);
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, config_ptr->debug ? GLFW_TRUE : GLFW_FALSE);
			glfwWindowHint(GLFW_CONTEXT_NO_ERROR, config_ptr->debug ? GLFW_FALSE : GLFW_TRUE);
			glfwWindowHint(GLFW_OPENGL_PROFILE, config_ptr->core_profile ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);

			// window settings
			glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
			glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
			glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
			glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		}
		else
			glfwDefaultWindowHints();

		// update creation parameters from previous window
		int X = -1, Y = -1;
		GLFWmonitor* monitor = 0;
		int refresh_rate = -1;
		if (glfw_window) {
			monitor = glfwGetWindowMonitor(glfw_window);
			if (monitor) {
				const GLFWvidmode* mode = glfwGetVideoMode(monitor);
				W = mode->width;
				H = mode->height;
				X = Y = 0;
				refresh_rate = mode->refreshRate;
			}
			else {
				glfwGetWindowSize(glfw_window, &W, &H);
				glfwGetWindowPos(glfw_window, &X, &Y);
			}
		}
		else {
			// full screen setting
			if (config_ptr && config_ptr->fullscreen_monitor != -1) {
				int n;
				GLFWmonitor** monitors = glfwGetMonitors(&n);
				if (config_ptr->fullscreen_monitor >= n)
					monitor = monitors[0];
				else
					monitor = monitors[config_ptr->fullscreen_monitor];
			}
		}
		GLFWwindow* new_glfw_window = glfwCreateWindow(W, H, title.c_str(), monitor, share->glfw_window);
		if (new_glfw_window == NULL)
			return false;
		if (glfw_window) {
			if (monitor == 0)
				glfwSetWindowPos(new_glfw_window, X, Y);
			glfwDestroyWindow(glfw_window);
		}
		glfw_window = new_glfw_window;
		glfwSetWindowUserPointer(glfw_window, this);
		glfwSetKeyCallback(glfw_window, &key_callback);
		glfwSetCursorPosCallback(glfw_window, &cursor_position_callback);
		glfwSetCursorEnterCallback(glfw_window, &cursor_enter_callback);
		glfwSetMouseButtonCallback(glfw_window, &mouse_button_callback);
		glfwSetScrollCallback(glfw_window, &scroll_callback);
		glfwSetWindowSizeCallback(glfw_window, resize_callback);
		return true;
	}
public:
	glfw_generic_window(int W, int H, const std::string& title, const cgv::render::render_config* config_ptr = 0, glfw_generic_window* share = 0) : cgv::gui::window("main")
	{
		glfw_window = 0;
		recreate_glfw_window(W, H, title, config_ptr, share);
		this->title = title;
	}
	/// this virtual method allows to pass application specific data for internal purposes
	void* get_user_data() const { return glfw_window; }
	/// show the %window. This needs to be called after creation to make the %window visible
	void show(bool modal)
	{
		glfwShowWindow(glfw_window);
	}
	/// hide the %window
	void hide()
	{
		glfwHideWindow(glfw_window);
	}
	/// returns the property declaration
	std::string get_property_declarations()
	{
		return
			glfw_base::get_property_declarations() + ";" + 
			"bg_clr_idx:uint32;gui:bool;icon:int32;menu:bool;state:string;W:uint32;H:uint32;" +
			cgv::base::base::get_property_declarations();
	}
	void ensure_menu_order()
	{
		std::cerr << "support for menu_order not implemented yet" << std::endl;
	}
	void ensure_dock_order()
	{
		std::cerr << "support for dock_order not implemented yet" << std::endl;
	}
	void show_status(bool do_show)
	{
		if (stats != do_show) {
			stats = do_show;
			on_set(&stats);
		}
	}
	void show_gui(bool do_show)
	{
		if (gui != do_show) {
			gui = do_show;
			on_set(&gui);
		}
	}
	void show_menu(bool do_show)
	{
		if (menu != do_show) {
			menu = do_show;
			on_set(&menu);
		}
	}
	bool self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return
			rh.reflect_member("title", title) &&
			rh.reflect_member("menu_order", menu_order) &&
			rh.reflect_member("dock_order", dock_order) &&
			rh.reflect_member("show_menu", menu) &&
			rh.reflect_member("show_gui", gui) &&
			rh.reflect_member("show_stats", stats);
	}
	void on_set(void* member_ptr)
	{
		if (member_ptr == &menu_order)
			ensure_menu_order();
		if (member_ptr == &dock_order)
			ensure_dock_order();
	}
	/// abstract interface for the setter implemented via the fltk_gui_group
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr)
	{
		// frame size changes
		if (property == "W" || property == "H") {
			int w, h, X,Y,W, H;
			glfwGetWindowSize(glfw_window, &w, &h);
			glfwGetWindowFrameSize(glfw_window, &X, &Y, &W, &H);
			W -= X;
			H -= Y;
			unsigned int new_value;
			cgv::type::get_variant(new_value, value_type, value_ptr);
			int new_w = w, new_h = h;
			if (property == "W")
				new_w = new_value - W + w;
			else
				new_h = new_value - H + h;
			if (new_w != w || new_h != h)
				glfwSetWindowSize(glfw_window, new_w, h);
			return true;
		}
		if (property == "w" || property == "h") {
			int w, h;
			glfwGetWindowSize(glfw_window, &w, &h);
			unsigned int new_value;
			cgv::type::get_variant(new_value, value_type, value_ptr);
			int new_w = w, new_h = h;
			if (property == "w")
				new_w = new_value;
			else
				new_h = new_value;
			if (new_w != w || new_h != h)
				glfwSetWindowSize(glfw_window, new_w, new_h);
			return true;
		}
		if (property == "title") {
			cgv::type::get_variant(title, value_type, value_ptr);
			glfwSetWindowTitle(glfw_window, title.c_str());
			return true;
		}
		if (property == "icon") {
			int i;
			cgv::type::get_variant(i, value_type, value_ptr);
			return false;
		}
		if (property == "x" || property == "y") {
			int x, y;
			glfwGetWindowPos(glfw_window, &x, &y);
			unsigned int new_value;
			cgv::type::get_variant(new_value, value_type, value_ptr);
			int new_x = x, new_y = y;
			if (property == "x")
				new_x = new_value;
			else
				new_y = new_value;
			if (new_x != x || new_y != y)
				glfwSetWindowPos(glfw_window, new_x, new_y);
			return true;
		}
		if (property == "cursor") {
			std::string cursor_name;
			cgv::type::get_variant(cursor_name, value_type, value_ptr);			
			glfwSetCursor(glfw_window, query_standard_cursors()[get_cursor_index(cursor_name)]);
		}
		else if (property == "status_info")
			show_status(cgv::type::variant<bool>::get(value_type, value_ptr));
		else if (property == "gui")
			show_gui(cgv::type::variant<bool>::get(value_type, value_ptr));
		else if (property == "menu")
			show_menu(cgv::type::variant<bool>::get(value_type, value_ptr));
		else if (property == "state") {
			std::string s = cgv::type::variant<std::string>::get(value_type, value_ptr);
			if (s == "minimized") {
				glfwIconifyWindow(glfw_window);
				return true;
			}
			if (s == "maximized") {
				glfwMaximizeWindow(glfw_window);
				return true;
			}
			if (s == "regular") {
				glfwRestoreWindow(glfw_window);
				return true;
			}
			if (s.substr(0, 10) == "fullscreen") {
				int ms = 0;
				int mo = 0;
				int nr = 0;
				if (s.length() > 10) {
					if (s[10] == '(' && s[s.length() - 1] == ')') {
						for (unsigned int i = 0; i < s.size() - 12; ++i) {
							int mi = s[11 + i] - '0';
							switch (mi) {
							case 1: ms += 1; ++nr; mo = 0; break;
							case 2: ms += 2; ++nr; mo = 1; break;
							case 3: ms += 4; ++nr; mo = 2; break;
							case 4: ms += 8; ++nr; mo = 3; break;
							case 5: ms += 16; ++nr; mo = 4; break;
							case 6: ms += 32; ++nr; mo = 5; break;
							}
						}
					}
				}
				if (nr == 0) {
					nr = 1;
					mo = 0;
				}
				int n;
				GLFWmonitor** monitors = glfwGetMonitors(&n);
				if (nr == 1) {
					if (mo >= n)
						mo = n - 1;
					GLFWmonitor* monitor = monitors[mo];
					const GLFWvidmode* mode = glfwGetVideoMode(monitor);
					glfwSetWindowMonitor(glfw_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
					return true;
				}
				/* todo: support fullscreen over multiple monitors
				else {
					int bit = 1;
					int x, y, w, h;
					for (int i = 0; i < n; ++i) {

						bit *= 2;
					}
				}
				*/
			}
		}
		else
			return base::set_void(property, value_type, value_ptr);
		return true;
	}
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr)
	{
		/** todo: implement getters */
		return false;
	}
	~glfw_generic_window()
	{
		glfwDestroyWindow(glfw_window);
	}
};
/*
class imgui_generic_window : public glfw_generic_window
{
protected:
	ImGuiContext* imgui_context;
public:
	imgui_generic_window(int W, int H, const std::string& title, const cgv::render::render_config* config_ptr = 0, glfw_generic_window* share = 0) :
		glfw_generic_window(W,H,title,config_ptr,share)
	{
		imgui_context = ImGui::CreateContext();
	}
	~imgui_generic_window() 
	{
		ImGui::DestroyContext(imgui_context);
	}
};
*/
class gl_viewer_window : public glfw_generic_window, public cgv::render::gl::gl_context, public cgv::render::gl::gl_performance_monitor
{
private:
	/// store whether we are in the draw method
	bool in_draw_method;
	///
	bool redraw_request;
protected:
	void on_resize(int w, int h)
	{
		make_current();
		resize_gl();
	}
public:
	/**@name context interface*/
	//@{
	/// draw some text at cursor position and update cursor position
	void draw_text(const std::string& text)
	{
		std::cout << text;
		std::cout.flush();
	}
	//void change_mode(int m);
	//static void idle_callback(void* gl_view);
public:
	//void create();
	//void destroy();
	gl_viewer_window(int W, int H, const std::string& title, const cgv::render::render_config* config_ptr = 0, glfw_generic_window* share = 0) :
		glfw_generic_window(W, H, title, config_ptr, share)
	{
		in_draw_method = false;
		redraw_request = false;
	}
	/// return the width of the window
	unsigned get_width() const
	{
		int width, height;
		glfwGetWindowSize(glfw_window, &width, &height);
		return width;
	}
	/// return the height of the window
	unsigned get_height() const
	{
		int width, height;
		glfwGetWindowSize(glfw_window, &width, &height);
		return height;
	}
	/// resize the context to the given dimensions
	void resize(unsigned int width, unsigned int height)
	{
		set("w", width);
		set("h", height);
	}
	/// clear the current context, typically used in multi-threaded rendering to allow usage of context in several threads
	void clear_current() const
	{
		glfwMakeContextCurrent(NULL);
	}
	/// return whether the context is currently in process of rendering
	bool in_render_process() const
	{
		return in_draw_method;
	}
	/// overload render pass to perform measurements
	void render_pass(cgv::render::RenderPass rp, cgv::render::RenderPassFlags rpf, void* ud)
	{
		start_task((int)rp);
		cgv::render::gl::gl_context::render_pass(rp, rpf, ud);
		if (enabled)
			glFinish();
		else
			glFlush();
		finish_task((int)rp);
	}
	/// return whether the context is created
	bool is_created() const
	{
		return glfw_window != 0;
	}
	/// return whether the context is current
	bool is_current() const
	{
		return glfw_window == glfwGetCurrentContext();
	}
	/// return the fltk mode determined from the context_config members
	//int determine_mode();
	/// set the context_config members from the current fltk mode
	//void synch_with_mode();
	/// recreate context based on current context config settings
	//bool recreate_context();
	/// make the current context current if possible
	bool make_current() const
	{
		glfwMakeContextCurrent(glfw_window);
		return is_current();
	}
	/// attach or detach (\c attach=false) an alpha buffer to the current frame buffer if not present
	void attach_alpha_buffer(bool attach) {}
	/// attach or detach (\c attach=false) depth buffer to the current frame buffer if not present
	void attach_depth_buffer(bool attach) {}
	/// attach or detach (\c attach=false) stencil buffer to the current frame buffer if not present
	void attach_stencil_buffer(bool attach) {}
	/// return whether the graphics card supports stereo buffer mode
	bool is_stereo_buffer_supported() const { return false; }
	/// attach or detach (\c attach=false) stereo buffer to the current frame buffer if not present
	void attach_stereo_buffer(bool attach) {}
	/// attach or detach (\c attach=false) accumulation buffer to the current frame buffer if not present
	void attach_accumulation_buffer(bool attach) {}
	/// attach or detach (\c attach=false) multi sample buffer to the current frame buffer if not present
	void attach_multi_sample_buffer(bool attach) {}
	/// the context will be redrawn when the system is idle again
	void post_redraw()
	{
		redraw_request = true;
	}
	/// the context will be redrawn right now. This method cannot be called inside the following methods of a drawable: init, init_frame, draw, finish_draw
	void force_redraw()
	{
		redraw_request = true;
	}
	/// enable the given font face with the given size in pixels
	void enable_font_face(cgv::media::font::font_face_ptr font_face, float font_size)
	{

	}
	/// return whether the graphics card supports quad buffer mode
	bool is_quad_buffer_supported() const 
	{
		return true;
	}
	//@}
};

/// create a window of the given type. Currently only the types "viewer with gui", "viewer" and "gui" are supported
window_ptr glfw_imgui_driver::create_window(int w, int h, const std::string& title, const std::string& window_type)
{
	window_ptr wp;
	if (window_type == "viewer")
		wp = window_ptr(new gl_viewer_window(w, h, title));
	else
		if (window_type == "generic")
			wp = window_ptr(new glfw_generic_window(w, h, title));
	if (wp.empty())
		return wp;
	windows.push_back(wp);
	return wp;
}
/// remove a window that has been destroyed
void glfw_imgui_driver::remove_window(window_ptr w)
{
	for (unsigned i = 0; i < windows.size(); ++i) {
		if (windows[i] == w) {
			windows.erase(windows.begin() + i);
			--i;
		}
	}
}
/// set the input focus to the given window
bool glfw_imgui_driver::set_focus(const_window_ptr w)
{
	glfwFocusWindow(const_cast<GLFWwindow*>(reinterpret_cast<const GLFWwindow*>(w->get_user_data())));
}
/// return the number of created windows
unsigned int glfw_imgui_driver::get_nr_windows()
{
	return (unsigned)windows.size();
}
/// return the i-th created window
window_ptr glfw_imgui_driver::get_window(unsigned int i)
{
	return windows[i];
}
/// run the main loop of the window system
bool glfw_imgui_driver::run()
{
	return false;
}
/// quit the application by closing all windows
void glfw_imgui_driver::quit(int exit_code)
{
	glfwTerminate();
	exit(exit_code);
}
/// copy text to the clipboard
void glfw_imgui_driver::copy_to_clipboard(const std::string& s)
{
	glfwSetClipboardString(0, s.c_str());

}
/// retreive text from clipboard
std::string glfw_imgui_driver::paste_from_clipboard()
{
	return glfwGetClipboardString(0);
}
//@}

/**@name some basic functionality */
//@{
/// ask the user with \c _question to select one of the \c answers, where \c default_answer specifies index of default answer
int glfw_imgui_driver::question(const std::string& _question, const std::vector<std::string>& answers, int default_answer)
{
	std::cerr << "glfw_imgui_driver::question function not yet implemented" << std::endl;
	return 0;
}
//! query the user for a text, where the second parameter is the default \c text as well as the returned text. 
/*! If \c password is true, the text is hidden. The function returns false if the user canceled the input of if no gui driver is available. */
bool glfw_imgui_driver::query(const std::string& question, std::string& text, bool password)
{
	std::cerr << "glfw_imgui_driver::query function not yet implemented" << std::endl;
	return true;
}
/// create a text editor
text_editor_ptr glfw_imgui_driver::create_text_editor(unsigned int w, unsigned int h, const std::string& title, int x, int y)
{

}
/// ask user for an open dialog that can select multiple files, return common path prefix and fill field of filenames
std::string glfw_imgui_driver::files_open_dialog(std::vector<std::string>& file_names, const std::string& title, const std::string& filter, const std::string& path)
{

}
/// ask user for a file to open
std::string glfw_imgui_driver::file_open_dialog(const std::string& title, const std::string& filter, const std::string& path)
{

}
/// ask user for a file to save
std::string glfw_imgui_driver::file_save_dialog(const std::string& title, const std::string& filter, const std::string& path)
{

}
//@}

/**@name threading based functionality */
//@{
//! lock the main thread of the gui from a child thread before any gui specific call.
/*! If lock is called several times, the child thread must call unlock the same number of times */
void glfw_imgui_driver::lock()
{
	mtx.lock();
}
/// unlock the main thread
void glfw_imgui_driver::unlock()
{
	mtx.unlock();
}
//! wake main thread.
/*! Ensures that main thead is not going to
	sleep any longer with the given message, that can be
	queried by the main thread with get_wakeup_message(). */
void glfw_imgui_driver::wake(const std::string& message)
{
	wakeup_message = message;
	glfwPostEmptyEvent();
}
/// return the message send by the thread that woke up the main thread with wake()
std::string glfw_imgui_driver::get_wakeup_message()
{
	return wakeup_message;
}
/// let the main thread sleep for the given number of seconds
void glfw_imgui_driver::sleep(float time_in_seconds)
{
	glfwWaitEventsTimeout(time_in_seconds);
}
//@}

/**@name gui elements */
//@{
base_ptr get_base_provider_generator(bool unregister = false)
{
	static base_ptr bpg_ptr;
	if (unregister) {
		if (bpg_ptr)
			unregister_object(bpg_ptr);
		bpg_ptr.clear();
	}
	else {
		if (!bpg_ptr) {
			bpg_ptr = base_ptr(new cgv::gui::base_provider_generator());
			register_object(base_ptr(bpg_ptr));
		}
	}
	return bpg_ptr;
}

/// process the gui declarations in the given gui file
bool glfw_imgui_driver::process_gui_file(const std::string& file_name)
{
	cgv::gui::base_provider_generator* bpg = get_base_provider_generator()->get_interface<cgv::gui::base_provider_generator>();
	return bpg->parse_gui_file(file_name);
}
/// add a new gui group to the given parent group
gui_group_ptr glfw_imgui_driver::add_group(gui_group_ptr parent, const std::string& label, const std::string& group_type, const std::string& options, const std::string& align);
/// add a newly created decorator to the parent group
base_ptr glfw_imgui_driver::add_decorator(gui_group_ptr parent, const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align);
/// add new button to the parent group
button_ptr glfw_imgui_driver::add_button(gui_group_ptr parent, const std::string& label, const std::string& options, const std::string& align);
/// add new view to the parent group
view_ptr glfw_imgui_driver::add_view(gui_group_ptr parent, const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align);
/// find a view in the group
view_ptr glfw_imgui_driver::find_view(gui_group_ptr parent, const void* value_ptr, int* idx_ptr);
/// add new control to the parent group
control_ptr glfw_imgui_driver::add_control(gui_group_ptr parent, const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align);
/// find a control in a group
control_ptr glfw_imgui_driver::find_control(gui_group_ptr parent, void* value_ptr, int* idx_ptr);
//@}

/**@name menu elements */
//@{
/// add a newly created decorator to the menu
base_ptr glfw_imgui_driver::add_menu_separator(const std::string& menu_path);
/// use the current gui driver to append a new button in the menu, where menu path is a '/' separated path
button_ptr glfw_imgui_driver::add_menu_button(const std::string& menu_path, const std::string& options);
/// use this to add a new control to the gui with a given value type, gui type and init options
cgv::data::ref_ptr<control<bool> > glfw_imgui_driver::add_menu_bool_control(const std::string& menu_path, bool& value, const std::string& options);
/// return the element of the given menu path
base_ptr glfw_imgui_driver::find_menu_element(const std::string& menu_path) const;
/// remove a single element from the gui
void glfw_imgui_driver::remove_menu_element(base_ptr);
//@}

struct menu_listener : public cgv::base::base, public cgv::base::registration_listener
{
	std::string get_type_name() const { return "menu_listener"; }
	void register_object(base_ptr object, const std::string& options)
	{
		menu_provider* mp = object->get_interface<menu_provider>();
		if (!mp) 
			return;
		mp->create_menu();
	}
	void unregister_object(base_ptr object, const std::string& options)
	{
		menu_provider* mp = object->get_interface<menu_provider>();
		if (!mp) 
			return;
		mp->destroy_menu();
	}
};

object_registration<glfw_imgui_driver> glfw_imgui_driver_registration("fltk driver");
object_registration<menu_listener> fml_reg("fltk menu driver");
