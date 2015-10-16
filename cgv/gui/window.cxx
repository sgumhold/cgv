#include "window.h"

namespace cgv {
	namespace gui {

window::window(const std::string& name) : gui_group(name) {}

std::string window::get_type_name() const
{
	return "window";
}

/// return the group that is managing the content of the window
gui_group_ptr window::get_inner_group()
{
	return gui_group_ptr(this);
}

/// dispatch a cgv event
bool window::dispatch_event(event&)
{
	return false;
}

	}
}
