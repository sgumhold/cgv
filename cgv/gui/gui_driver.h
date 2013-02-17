#pragma once

#include <string>
#include <cgv/base/base.h>
#include <cgv/signal/signal.h>
#include <cgv/data/ref_counted.h>
#include "button.h"
#include "gui_group.h"
#include "view.h"
#include "control.h"
#include "window.h"
#include "text_editor.h"
#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// abstract %base class for %gui drivers
class CGV_API gui_driver : public cgv::base::base, public cgv::base::driver
{
public:
	/**@name application management*/
	//@{
	/// create a window of the given type. Currently only the types "viewer with gui", "viewer" and "gui" are supported
	virtual window_ptr create_window(int w, int h, const std::string& title, const std::string& window_type) = 0;
	/// set the input focus to the given window
	virtual bool set_focus(const_window_ptr) = 0;
	/// return the number of created windows
	virtual unsigned int get_nr_windows() = 0;
	/// return the i-th created window
	virtual window_ptr get_window(unsigned int i) = 0;
	/// run the main loop of the window system
	virtual bool run() = 0;
	/// quit the application by closing all windows
	virtual void quit(int exit_code) = 0;
	/// copy text to the clipboard
	virtual void copy_to_clipboard(const std::string& s) = 0;
	/// retreive text from clipboard
	virtual std::string paste_from_clipboard() = 0;
	//@}

	/**@name some basic functionality */
	//@{
	/// ask the user with \c _question to select one of the \c answers, where \c default_answer specifies index of default answer
	virtual int question(const std::string& _question, const std::vector<std::string>& answers, int default_answer = -1) = 0;
	//! query the user for a text, where the second parameter is the default \c text as well as the returned text. 
	/*! If \c password is true, the text is hidden. The function returns false if the user canceled the input of if no gui driver is available. */
	virtual bool query(const std::string& question, std::string& text, bool password = false) = 0;
	/// create a text editor
	virtual text_editor_ptr create_text_editor(unsigned int w, unsigned int h, const std::string& title, int x, int y) = 0;
	/// ask user for an open dialog that can select multiple files, return common path prefix and fill field of filenames
	virtual std::string files_open_dialog(std::vector<std::string>& file_names, const std::string& title, const std::string& filter, const std::string& path) = 0;
	/// ask user for a file to open
	virtual std::string file_open_dialog(const std::string& title, const std::string& filter, const std::string& path) = 0;
	/// ask user for a file to save
	virtual std::string file_save_dialog(const std::string& title, const std::string& filter, const std::string& path) = 0;
	//@}

	/**@name threading based functionality */
	//@{
	//! lock the main thread of the gui from a child thread before any gui specific call.
	/*! If lock is called several times, the child thread must call unlock the same number of times */
	virtual void lock() = 0;
	/// unlock the main thread
	virtual void unlock() = 0;
	//! wake main thread.
	/*! Ensures that main thead is not going to 
	    sleep any longer with the given message, that can be
		queried by the main thread with get_wakeup_message(). */
	virtual void wake(const std::string& message = "") = 0;
	/// return the message send by the thread that woke up the main thread with wake()
	virtual std::string get_wakeup_message() = 0;
	/// let the main thread sleep for the given number of seconds
	virtual void sleep(float time_in_seconds) = 0;
	//@}

	/**@name gui elements */
	//@{
	/// process the gui declarations in the given gui file
	virtual bool process_gui_file(const std::string& file_name) = 0;
	/// add a new gui group to the given parent group
	virtual gui_group_ptr add_group(gui_group_ptr parent, const std::string& label, const std::string& group_type, const std::string& options, const std::string& align) = 0;
	/// add a newly created decorator to the parent group
	virtual base_ptr add_decorator(gui_group_ptr parent, const std::string& label, const std::string& decorator_type, const std::string& options, const std::string& align) = 0;
	/// add new button to the parent group
	virtual button_ptr add_button(gui_group_ptr parent, const std::string& label, const std::string& options, const std::string& align) = 0;
	/// add new view to the parent group
	virtual view_ptr add_view(gui_group_ptr parent, const std::string& label, const void* value_ptr, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align) = 0;
	/// find a view in the group
	virtual view_ptr find_view(gui_group_ptr parent, const void* value_ptr, int* idx_ptr) = 0;
	/// add new control to the parent group
	virtual control_ptr add_control(gui_group_ptr parent, const std::string& label, void* value_ptr, abst_control_provider* acp, const std::string& value_type, const std::string& gui_type, const std::string& options, const std::string& align) = 0;
	/// find a control in a group
	virtual control_ptr find_control(gui_group_ptr parent, void* value_ptr, int* idx_ptr) = 0;
	//@}

	/**@name menu elements */
	//@{
	/// add a newly created decorator to the menu
	virtual base_ptr add_menu_separator(const std::string& menu_path) = 0;
	/// use the current gui driver to append a new button in the menu, where menu path is a '/' separated path
	virtual button_ptr add_menu_button(const std::string& menu_path, const std::string& options) = 0;
	/// use this to add a new control to the gui with a given value type, gui type and init options
	virtual data::ref_ptr<control<bool> > add_menu_bool_control(const std::string& menu_path, bool& value, const std::string& options) = 0;
	/// return the element of the given menu path
	virtual base_ptr find_menu_element(const std::string& menu_path) const = 0;
	/// remove a single element from the gui
	virtual void remove_menu_element(base_ptr) = 0;
	//@}
};

/// ref counted pointer to driver
typedef data::ref_ptr<gui_driver> gui_driver_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API data::ref_ptr<gui_driver>;
#endif

/// return the currently registered gui driver or an empty pointer if non has been registered
extern CGV_API gui_driver_ptr get_gui_driver();

//! register a new gui driver. 
/*! This overwrites a previously registered gui driver and should 
    be called in the on_register method of the driver*/
extern CGV_API void register_gui_driver(gui_driver_ptr _driver);

/// a signal that is emitted when a gui driver is registered
extern CGV_API cgv::signal::signal<gui_driver_ptr>& on_gui_driver_registration();

	}
}

#include <cgv/config/lib_end.h>
