#pragma once

#include <cgv/gui/gui_driver.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// implementation of the gui driver for fltk
class CGV_API win32_gui_driver : public gui_driver
{
public:
	/// construct a driver
	win32_gui_driver() {}
	/**@name application management*/
	//@{
	/// create a window of the given type. Currently only the types "viewer with gui", "viewer" and "gui" are supported
	window_ptr create_window(int w, int h, const std::string& title, const std::string& window_type) { return window_ptr(); }
	/// set the input focus to the given window
	bool set_focus(const_window_ptr) { return false; }
	/// return the number of created windows
	unsigned int get_nr_windows() { return 1; }
	/// return the i-th created window
	window_ptr get_window(unsigned int i) { return window_ptr(); }
	/// run the main loop of the window system
	bool run() { return false; }
	/// quit the application by closing all windows
	void quit() {}
	//@}

	/**@name some basic functionality */
	//@{
	/// create a text editor
	text_editor_ptr create_text_editor(unsigned int w, unsigned int h, const std::string& title, int x, int y) { return text_editor_ptr(); }
	/// ask user for a file to open
	std::string file_open_dialog(const std::string& title, const std::string& filter, const std::string& path);
	/// ask user for a file to save
	std::string file_save_dialog(const std::string& title, const std::string& filter, const std::string& path);
	//@}

	/**@name threading based functionality */
	//@{
	/** lock the main thread of the gui from a child thread before any 
	    gui specific call. If lock is called several times, the child
		thread must call unlock the same number of times */
	void lock() {}
	/// unlock the main thread
	void unlock() {}
	/** wake the main thread to ensure that it is not going to 
	    sleep any longer with the given message, that can be
		queried by the main thread with get_wakeup_message(). */
	void wake(const std::string& message) {}
	/// return the message send by the thread that woke up the main thread with wake()
	std::string get_wakeup_message() { return ""; }
	/// let the main thread sleep for the given number of seconds
	void sleep(float time_in_seconds) {}
	//@}

	/**@name gui elements */
	//@{
	/// process the gui declarations in the given gui file
	bool process_gui_file(const std::string& file_name) { return false; }
	/// add a new gui group to the given parent group
	gui_group_ptr add_group(gui_group_ptr parent, const std::string& label, const std::string& group_type, const std::string& options, const std::string& align) { return gui_group_ptr(); }
	/// add a newly created decorator to the parent group
	base_ptr add_decorator(gui_group_ptr parent, const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align) { return base_ptr(); }
	/// add new button to the parent group
	button_ptr add_button(gui_group_ptr parent, const std::string& label, const std::string& options, const std::string& align) { return button_ptr(); }
	/// add new view to the parent group
	view_ptr add_view(gui_group_ptr parent, const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align) { return view_ptr(); }
	/// find a view in the group
	view_ptr find_view(gui_group_ptr parent, const void* value_ptr, int* idx_ptr) { return view_ptr(); }
	/// add new control to the parent group
	control_ptr add_control(gui_group_ptr parent, const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align) { return control_ptr(); }
	/// find a control in a group
	control_ptr find_control(gui_group_ptr parent, void* value_ptr, int* idx_ptr) { return control_ptr(); }
	//@}

	/**@name menu elements */
	//@{
	/// add a newly created decorator to the menu
	base_ptr add_menu_separator(const std::string& menu_path) { return base_ptr(); }
	/// use the current gui driver to append a new button in the menu, where menu path is a '/' separated path
	button_ptr add_menu_button(const std::string& menu_path, const std::string& options) { return button_ptr(); }
	/// use this to add a new control to the gui with a given value type, gui type and init options
	cgv::data::ref_ptr<control<bool> > add_menu_bool_control(const std::string& menu_path, bool& value, const std::string& options) {return cgv::data::ref_ptr<control<bool> >(); }
	/// return the element of the given menu path
	base_ptr find_menu_element(const std::string& menu_path) const { return base_ptr(); }
	/// remove a single element from the gui
	void remove_menu_element(base_ptr) {}
	//@}
};

	}
}

#include <cgv/config/lib_end.h>