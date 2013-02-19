#pragma once

#include <cgv/gui/button.h>
#include <cgv/gui/control.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// derive from this class to provide a gui to the current viewer
class CGV_API menu_provider //: public cgv::signal::tacker
{
protected:
	/**@name creation of menu entries*/
	//@{
	/// add a newly created decorator to the group
	base::base_ptr add_menu_separator(const std::string& menu_path);
	/// use the current gui driver to append a new button in the menu, where menu path is a '/' separated path
	button_ptr add_menu_button(const std::string& menu_path, const std::string& options = "");
	/// use this to add a new control to the gui with a given value type, gui type and init options
	data::ref_ptr<control<bool> > add_menu_bool_control(const std::string& menu_path, bool& value, const std::string& options = "");
	//@}

	/**@name update menu*/
	//@{
	/// return the element of the given menu path
	base::base_ptr find_menu_element(const std::string& menu_path) const;
	/// remove a single element from the gui
	void remove_menu_element(base::base_ptr);
	//@}
public:
	/// default construction
	menu_provider();
	/// ensure to remove posted recreation callbacks
	~menu_provider();
	/// you must overload this for menu creation
	virtual void create_menu() = 0;
	/// you must overload this for to remove all elements from the menu again
	virtual void destroy_menu() = 0;
};

	}
}

#include <cgv/config/lib_end.h>