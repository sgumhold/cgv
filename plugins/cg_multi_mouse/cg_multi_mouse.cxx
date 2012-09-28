#include <multi_mouse_fltk.h>
#include <multi_mouse.h>
#include <cgv/gui/application.h>
#include <cgv/gui/gui_driver.h>
#include <iostream>

using namespace std;
using namespace cgv::base;
using namespace cgv::gui;

struct cgv_multi_mouse_handler : public multi_mouse_handler
{
	/// callback for handling event. Return whether this event has been processed
	bool handle_multi_mouse(const multi_mouse_event& mme)
	{
		for (unsigned i=0; i<application::get_nr_windows(); ++i)
			if (application::get_window(i)->dispatch_event(const_cast<multi_mouse_event&>(mme)))
				return true;
		return false;
	}
};

void register_multi_mouse(gui_driver_ptr driver)
{
	if (driver->get_type_name() != "fltk_driver") {
		cerr << "cg_multi_mouse plugin only implemented for fltk_driver from cg_fltk plugin" << endl;
		return;
	}
	static cgv_multi_mouse_handler cmmh;
	register_multi_mouse_handler(&cmmh);
	enable_multi_mouse();
	for (unsigned i=0; i<application::get_nr_windows(); ++i)
		attach_multi_mouse_to_fltk((fltk::Window*)application::get_window(i)->get_user_data());
}

struct multi_mouse_reg
{
	multi_mouse_reg(const char* name)
	{
		gui_driver_ptr driver = get_gui_driver();
		if (!driver)
			connect(on_gui_driver_registration(),register_multi_mouse);
		else
			register_multi_mouse(driver);
	}
};

#include <cgv/base/register.h>

#if defined(CGV_GUI_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#ifdef CG_MULTI_MOUSE_EXPORTS
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>

extern CGV_API multi_mouse_reg mm_reg("");
