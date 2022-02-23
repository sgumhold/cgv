#include <cgv/base/register.h>
#include <cgv/gui/application.h>
#include <cgv/gui/gui_driver.h>
#include <cgv/gui/base_provider_generator.h>
#include <cgv/utils/file.h>
#include <cgv/render/context.h>
#include <cgv/base/console.h>
#include <cgv/utils/tokenizer.h>
#include <cgv/utils/advanced_scan.h>

using namespace cgv::base;
using namespace cgv::gui;
using namespace cgv::utils;

void generate_gui(gui_driver_ptr d)
{
	window_ptr w = application::create_window(1280,768,"CGV Viewer");
	w->show();
	register_object(w, "register viewer window");
}

int main(int argc, char** argv)
{
	cgv::render::render_config_ptr rcp = cgv::render::get_render_config();
	if (!rcp->is_property("debug"))
		std::cout << "no debug property ;(" << std::endl;
	cgv::base::register_prog_name(argv[0]);
	connect(on_gui_driver_registration(),generate_gui);
	disable_registration_event_cleanup();
	enable_permanent_registration();
	enable_registration();
	register_object(console::get_console());

	// try to process command line arguments and store unknown and name and type commands where no object could be found
	// process_command_line_args(argc, argv);
	std::vector<command_info> unprocessed;
	std::vector<int> unknown;
	bool loaded_config = false;
	unsigned ai;
	for (ai = 1; (int)ai < argc; ++ai) {
		command_info info;
		cgv::base::analyze_command(cgv::utils::token(argv[ai], argv[ai] + std::string((const char*)argv[ai]).length()), true, &info);
		switch (info.command_type) {
		case CT_UNKNOWN:
			unknown.push_back(ai);
			break;
		case CT_CONFIG:
			loaded_config = true;
		case CT_GUI:
		case CT_SHOW:
		case CT_PLUGIN:
			process_command(info);
			break;
		case CT_TYPE:
		case CT_NAME:
			if (!process_command(info))
				unprocessed.push_back(info);
			break;
		}
	}

	if (!loaded_config) {
		std::string cfg_file_name = cgv::utils::file::drop_extension(argv[0]) + ".cfg";
		if (cgv::utils::file::exists(cfg_file_name))
			process_config_file(cfg_file_name);
	}

	enable_registration_event_cleanup();

	// process so far unprocessed command line arguments
	for (ai = 0; ai<(int)unprocessed.size(); ++ai)
		cgv::base::process_command(unprocessed[ai]);

	// attempt to read volume file named by otherwise unknown arguments
	if (!unknown.empty()) {
		std::vector<std::string> args;
		for (ai = 0; ai < (int)unknown.size(); ++ai)
			args.push_back(argv[unknown[ai]]);
		unsigned no = get_nr_permanently_registered_objects();
		for (ai = 0; ai < no && !args.empty(); ++ai) {
			cgv::base::base_ptr object = get_permanently_registered_object(ai);
			cgv::base::argument_handler* ah = object->get_interface < cgv::base::argument_handler >();
			if (ah)
				ah->handle_args(args);
		}
		for (ai = 0; ai < args.size(); ++ai)
			std::cerr << "WARNING: unknown command line argument '" << args[ai] << "'" << std::endl;
	}
	bool res = application::run();
	unregister_all_objects();
	return res;
}
