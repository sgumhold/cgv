#pragma once

#include <cgv/base/base.h>
#include <cgv/gui/gui_driver.h>

#include "lib_begin.h"

using namespace cgv::gui;

namespace fltk {
	class Menu;
}

/// implementation of the gui driver for fltk
class CGV_API fltk_driver : public gui_driver
{
protected:
	/// keep track of the created windows
	std::vector<window_ptr> windows;
	/// store the last wakeup message
	std::string wakeup_message;
public:
	/// remove a window that has been destroyed
	void remove_window(window_ptr w);
	/// register driver in cgv frame work
	void on_register();
	/// returns "fltk_driver"
	std::string get_type_name() const;
	/**@name application management*/
	//@{
	/// create a window of the given type. Currently only the types "viewer with gui", "viewer" and "gui" are supported
	window_ptr create_window(int w, int h, const std::string& title, const std::string& window_type);
	/// set the input focus to the given window
	bool set_focus(const_window_ptr);
	/// return the number of created windows
	unsigned int get_nr_windows();
	/// return the i-th created window
	window_ptr get_window(unsigned int i);
	/// run the main loop of the window system
	bool run();
	/// quit the application by closing all windows
	void quit(int exit_code);
	/// copy text to the clipboard
	void copy_to_clipboard(const std::string& s);
	/// retreive text from clipboard
	std::string paste_from_clipboard();
	//@}

	/**@name some basic functionality */
	//@{
	/// create a text editor
	text_editor_ptr create_text_editor(unsigned int w, unsigned int h, const std::string& title, int x, int y);
	/// ask the user with \c _question to select one of the \c answers, where \c default_answer specifies index of default answer
	int question(const std::string& _question, const std::vector<std::string>& answers, int default_answer = -1);
	//! query the user for a text, where the second parameter is the default \c text as well as the returned text. 
	/*! If \c password is true, the text is hidden. The function returns false if the user canceled the input of if no gui driver is available. */
	bool query(const std::string& question, std::string& text, bool password = false);
	/// ask user for an open dialog that can select multiple files, return common path prefix and fill field of filenames
	std::string files_open_dialog(std::vector<std::string>& file_names, const std::string& title, const std::string& filter, const std::string& path);
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
	void lock();
	/// unlock the main thread
	void unlock();
	/** wake the main thread to ensure that it is not going to 
	    sleep any longer with the given message, that can be
		queried by the main thread with get_wakeup_message(). */
	void wake(const std::string& message);
	/// return the message send by the thread that woke up the main thread with wake()
	std::string get_wakeup_message();
	/// let the main thread sleep for the given number of seconds
	void sleep(float time_in_seconds);
	//@}

	/**@name gui elements */
	//@{
	/// process the gui declarations in the given gui file
	bool process_gui_file(const std::string& file_name);
	/// add a new gui group to the given parent group
	virtual gui_group_ptr add_group(gui_group_ptr parent, const std::string& label, const std::string& group_type, const std::string& options, const std::string& align);
	/// add a newly created decorator to the parent group
	virtual base_ptr add_decorator(gui_group_ptr parent, const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align);
	/// add new button to the parent group
	virtual button_ptr add_button(gui_group_ptr parent, const std::string& label, const std::string& options, const std::string& align);
	/// add new view to the parent group
	virtual view_ptr add_view(gui_group_ptr parent, const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align);
	/// find a view in the group
	virtual view_ptr find_view(gui_group_ptr parent, const void* value_ptr, int* idx_ptr);
	/// add new control to the parent group
	virtual control_ptr add_control(gui_group_ptr parent, const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align);
	/// find a control in a group
	virtual control_ptr find_control(gui_group_ptr parent, void* value_ptr, int* idx_ptr);
	//@}

	/**@name menu elements */
	//@{
	/// analyze a menu path description, search for the menu that will contain the path and split path in path and item name
	fltk::Menu* resolve_menu_path(const std::string& menu_path, std::string& path, std::string& name, bool ensure_created) const;
	/// add a newly created decorator to the menu
	base_ptr add_menu_separator(const std::string& menu_path);
	/// use the current gui driver to append a new button in the menu, where menu path is a '/' separated path
	button_ptr add_menu_button(const std::string& menu_path, const std::string& options);
	/// use this to add a new control to the gui with a given value type, gui type and init options
	cgv::data::ref_ptr<control<bool> > add_menu_bool_control(const std::string& menu_path, bool& value, const std::string& options);
	/// return the element of the given menu path
	base_ptr find_menu_element(const std::string& menu_path) const;
	/// remove a single element from the gui
	void remove_menu_element(base_ptr);
	//@}
};

extern CGV_API void ensure_lock();

#include <cgv/config/lib_end.h>