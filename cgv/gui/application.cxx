#include "application.h"
#include "gui_driver.h"
#include <iostream>
#include <stdlib.h>

namespace cgv {
	namespace gui {

window_ptr application::create_window(int w, int h, const std::string& title, const std::string& window_type)
{
	if (get_gui_driver().empty())
		return window_ptr();
	return get_gui_driver()->create_window(w,h,title,window_type);
}

bool application::set_focus(const_window_ptr w)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return false;
	return d->set_focus(w);
}

unsigned int application::get_nr_windows()
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return 0;
	return d->get_nr_windows();
}

window_ptr application::get_window(unsigned int i)
{
	if (get_gui_driver().empty())
		return window_ptr();
	return get_gui_driver()->get_window(i);
}

bool application::run()
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty()) {
		std::cerr << "\nSorry, but no cgv::gui-driver available.\n"
			<< "To get one, compile project 'cg_fltk' and add 'plugin:cg_fltk.cgv'\n"
			<< "to the argument list when starting the fltk_viewer\n"
			<< "(in Visual Studio right click you project and get to properties,\n"
			<< " select 'configuration settings' -> 'debugging' -> command line arguments')" << std::endl;
		return false;
	}
	return d->run();
}

void application::quit(int exit_code)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		exit(exit_code);
	d->quit(exit_code);
}

/// copy text to the clipboard
void application::copy_to_clipboard(const std::string& s)
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		exit(0);
	d->copy_to_clipboard(s);
}

/// retreive text from clipboard
std::string application::paste_from_clipboard()
{
	gui_driver_ptr d = get_gui_driver();
	if (d.empty())
		return "";
	return d->paste_from_clipboard();
}

	}
}
