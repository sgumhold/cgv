#include <cgv/gui/gui_driver.h>
#include "menu_provider.h"

namespace cgv {
	namespace gui {

/// add a newly created decorator to the group
base_ptr menu_provider::add_menu_separator(const std::string& menu_path)
{
	if (get_gui_driver())
		return get_gui_driver()->add_menu_separator(menu_path);
	return base_ptr();
}

/// use the current gui driver to append a new button in the menu, where menu path is a '/' separated path
button_ptr menu_provider::add_menu_button(const std::string& menu_path, const std::string& options)
{
	if (get_gui_driver())
		return get_gui_driver()->add_menu_button(menu_path, options);
	return button_ptr();
}

/// use this to add a new control to the gui with a given value type, gui type and init options
data::ref_ptr<control<bool> > menu_provider::add_menu_bool_control(const std::string& menu_path, bool& value, const std::string& options)
{
	if (get_gui_driver())
		return get_gui_driver()->add_menu_bool_control(menu_path, value, options);
	return data::ref_ptr<control<bool> >();
}
/// return the element of the given menu path
base::base_ptr menu_provider::find_menu_element(const std::string& menu_path) const
{
	if (get_gui_driver())
		return get_gui_driver()->find_menu_element(menu_path);
	return base::base_ptr();
}
/// remove a single element from the gui
void menu_provider::remove_menu_element(base::base_ptr bp)
{
	if (get_gui_driver())
		get_gui_driver()->remove_menu_element(bp);
}
/// default construction
menu_provider::menu_provider()
{
}
/// ensure to remove posted recreation callbacks
menu_provider::~menu_provider()
{
}

	}
}
