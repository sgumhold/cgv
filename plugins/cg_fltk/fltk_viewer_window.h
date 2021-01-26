#pragma once

#include <cgv/gui/event_handler.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/window.h>
#include "DockableGroup.h"
#include "fltk_tab_group.h"
#include "fltk_gl_view.h"

#ifdef WIN32
#pragma warning (disable:4311)
#endif
#include <fltk/Window.h>
#include <fltk/Menu.h>
#ifdef WIN32
#pragma warning (default:4311)
#endif

namespace fltk {
	class ItemGroup;
	class MenuBar;
	class TiledGroup;
};

#include "lib_begin.h"

enum WindowState { 
	WS_REGULAR = 0, 
	WS_MINIMIZED = 1, 
	WS_MAXIMIZED = 2, 
	WS_FULLSCREEN = 3
};

enum MonitorSelection
{
	MS_MONITOR_CURRENT = 0,
	MS_MONITOR_1 = 1,
	MS_MONITOR_2 = 2,
	MS_MONITOR_3 = 4,
	MS_MONITOR_4 = 8,
	MS_MONITOR_ALL = 15,
};

/** the fltk_viewer_window is implemented with the help of a fltk_gl_view
    and a tab_group that incorporates guis in registration events. */
class CGV_API fltk_viewer_window : 
	public cgv::gui::window,
	public fltk_gui_group,
	public fltk::Window,
	public cgv::gui::provider
{
protected:
	void on_tab_group_selection_change(base_ptr, bool);
public:
	/// construct application
	fltk_viewer_window(int w, int h, const std::string& _title);
	/// show the window. This needs to be called after creation to make the window visible
	void show(bool modal);
	/// hide the window
	void hide();
	/// returns fltk_viewer_window
	std::string get_type_name() const;
	/// return a fltk::Widget pointer that can be cast to a fltk::Window
	void* get_user_data() const;
	/// passes update over to the fltk_gui_group if it exists
	void update();
	///
	void on_register();
	/// returns the property declaration
	std::string get_property_declarations();
	/// abstract interface for the setter 
	bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter 
	bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// overload and use the fltk_gui_group implementation
	unsigned int remove_child(base_ptr child);
	/// overload and use the fltk_gui_group implementation
	void remove_all_children();
	/// dispatch a cgv event to the gl view
	bool dispatch_event(cgv::gui::event& e);
	/// overload fltk handle method
	int handle(int event);
	/// return the view
	fltk_gl_view_ptr get_view() { return view; }
	/// put default sizes into dimension fields and set inner_group to be active
	void prepare_new_element(cgv::gui::gui_group_ptr ggp, int& x, int& y, int& w, int& h);
	/// align last element and add element to group
	void finalize_new_element(cgv::gui::gui_group_ptr ggp, const std::string& align, cgv::base::base_ptr element);
	/// interface of adding an object
	void register_object(cgv::base::base_ptr object, const std::string& options);
	/// interface for removing an object
	void unregister_object(cgv::base::base_ptr object, const std::string& options);
	/// set a different window state
	void set_window_state(WindowState ws, MonitorSelection ms, bool update_control = true);
	/// return the current window state
	WindowState get_window_state() const;
	/// show the gui in case there has been some gui::provider s registered
	void show_gui(bool update_control = true);
	/// show the menu bar in case there are any entries
	void show_menu(bool update_control = true);
	/// return whether the gui is shown
	bool gui_shown() const;
	/// return whether the menu bar is shown
	bool menu_shown() const;
	/// hide the gui
	void hide_gui(bool update_control = true);
	/// hide the menu bar
	void hide_menu(bool update_control = true);
	/// return a path in the main menu to select the gui
	std::string get_menu_path() const;
	/// return a shortcut to activate the gui without menu navigation
	cgv::gui::shortcut get_shortcut() const;
	/// you must overload this for gui creation
	void create_gui();
	/// return a pointer to the main menu
	fltk::Menu* get_menu();
protected:
	/// ensure that the menu is in the specific order
	void ensure_menu_order();
	/// store the menu order
	std::string menu_order;
	///
	unsigned int get_dock_idx(int i) const;
	/// ensure that the menu is in the specific order
	void ensure_dock_order();
	/// store the menu order
	std::string dock_order;
	/// store the current window state
	WindowState window_state, old_window_state;
	/// monitors covered in fullscreen
	MonitorSelection fullscreen_monitors;
	/// temporarily store the window size before going to fullscreen or maximize
	int old_x, old_y, old_w, old_h;
	/// whether to show gui and menu bar
	bool gui_visible, menu_visible;
	/// store the factories
	std::vector<base_ptr> factories;
	/// 
	::fltk::ItemGroup* create_item_group;
	/** the main_group is used to split the space between menu bar
	    and status bar into fltk_gl_view and the tab group to which 
		 the guis of registered objects are added */
	DockableGroup* main_group;
	/// manages the main menu
	fltk::MenuBar* menu;
	/// the gl view is used as central widget
	fltk_gl_view_ptr view;
	/// used to manage guis of registered objects
	fltk_tab_group_ptr tab_group;

	/// title of the window
	std::string title;

	static void create_cb(fltk::Widget* w, void* user_data);

	static void menu_cb(fltk::Widget* w, void* user_data);
	
	bool ws_change_cb(cgv::gui::control<WindowState>& c);
	bool ms_change_cb(cgv::gui::control<MonitorSelection>& c);
	void menu_change_cb();
	void gui_change_cb();
};

#include <cgv/config/lib_end.h>