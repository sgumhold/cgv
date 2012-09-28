#include "gui_driver.h"

namespace cgv {
	namespace gui {

gui_driver_ptr& ref_gui_driver()
{
	static gui_driver_ptr current_driver;
	return current_driver;
}

gui_driver_ptr get_gui_driver()
{
	return ref_gui_driver();
}

void register_gui_driver(gui_driver_ptr _driver)
{
	ref_gui_driver() = _driver;
	on_gui_driver_registration()(_driver);
}

/// a signal that is emitted when a gui driver is registered
cgv::signal::signal<gui_driver_ptr>& on_gui_driver_registration()
{
	static cgv::signal::signal<gui_driver_ptr> s;
	return s;
}


	}
}
