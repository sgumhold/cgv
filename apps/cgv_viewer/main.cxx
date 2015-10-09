#include <cgv/base/register.h>
#include <cgv/gui/application.h>
#include <cgv/gui/gui_driver.h>
#include <cgv/gui/base_provider_generator.h>
#include <cgv/utils/file.h>
#include <cgv/base/console.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/advanced_scan.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::utils;

void generate_gui(gui_driver_ptr d)
{
	window_ptr w = application::create_window(1280,768,"cgv 3d viewer");
	w->show();
	register_object(w, "register viewer window");
}

int main(int argc, char** argv)
{
	cgv::base::register_prog_name(argv[0]);
	connect(on_gui_driver_registration(),generate_gui);
	disable_registration_event_cleanup();
	enable_permanent_registration();
	enable_registration();
	register_object(console::get_console());
	process_command_line_args(argc, argv);
	enable_registration_event_cleanup();
	bool res = application::run();
	unregister_all_objects();
	return res;
}
