#include "help_menu_entry.h"

#include <cgv/gui/gui_driver.h>

namespace cgv {
	namespace gui {

void help_menu_entry::on_register() {
	// TODO: Prevent double registration/creation of window.

	auto driver = cgv::gui::get_gui_driver();
	wnd = driver->create_window(400, 300, "Help: " + get_name(), "generic");
	std::string value = "scroll_group";
	wnd->set_void("group", "string", &value);

	for(const auto& section : sections) {
		switch(section.type) {
		case SectionType::kHeading1:
			wnd->add_decorator(section.content, "heading", "level=0", "");
			break;
		case SectionType::kHeading2:
			wnd->add_decorator(section.content, "heading", "level=1", "");
			break;
		case SectionType::kHeading3:
			wnd->add_decorator(section.content, "heading", "level=2", "");
			break;
		case SectionType::kHeading4:
			wnd->add_decorator(section.content, "heading", "level=3", "");
			break;
		case SectionType::kText:
			wnd->add_decorator(section.content, "text", "", "");
			break;
		case SectionType::kItems:
		{
			std::vector<cgv::utils::token> tokens;
			cgv::utils::bite_all(cgv::utils::tokenizer(section.content).set_skip("'\"", "'\"", "\\\\").set_ws(";"), tokens);

			if(tokens.empty())
				break;

			for(const auto& token : tokens) {
				std::string text = "  -\t" + to_string(token);
				// TODO: Prevent y-spacing between items.
				wnd->add_decorator(text, "text", "", "");
			}

			break;
		}
		case SectionType::kKeyBindings:
		{
			std::vector<cgv::utils::token> tokens;
			cgv::utils::bite_all(cgv::utils::tokenizer(section.content).set_skip("'\"", "'\"", "\\\\").set_ws(";"), tokens);

			if(tokens.empty())
				break;

			std::string group_options = "layout=table;border-style=framed;cols=2";
			group_options += ";rows=" + std::to_string(tokens.size());

			auto group = wnd->add_group("", "layout_group", group_options, "");
			groups.push_back(group);

			for(const auto& token : tokens) {
				std::vector<cgv::utils::token> sides;
				cgv::utils::bite_all(cgv::utils::tokenizer(token).set_skip("'\"", "'\"", "\\\\").set_ws("="), sides);

				if(sides.size() != 2) {
					//if(report_error)
					//	std::cerr << "property assignment >" << to_string(toks[i]).c_str() << "< does not match pattern lhs=rhs" << std::endl;
					continue;
				}

				std::string lhs = cgv::utils::trim(to_string(sides[0]), " ");
				std::string rhs = cgv::utils::trim(to_string(sides[1]), "'");

				group->add_decorator("  " + lhs, "text", "w=100;border=true", "S");
				group->add_decorator(rhs, "text", "border=true", "sSxX");
			}

			break;
		}
		default:
			break;
		}
	}

	wnd->show();
	
	// TODO: Do we need to register the window?
	//cgv::base::register_object(wnd, "register help window");

	// TODO: Do we need to unregister/clear or whatever the window before we unregister this instance?
	//driver->remove_window(wnd);
	//wnd.clear();

	cgv::base::unregister_object(this);
}

void help_menu_entry::unregister() {
	// TODO: Do potential unregister actions.
}

	}
}
