#include "window.h"

namespace cgv {
	namespace gui {

window::window(const std::string& name) : gui_group(name) {}

std::string window::get_type_name() const
{
	return "window";
}

/// dispatch a cgv event
bool window::dispatch_event(event&)
{
	return false;
}

	}
}
