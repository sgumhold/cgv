#pragma once

#include <cgv/gui/dialog.h>
#include <cgv/gui/provider.h>

namespace cgv {
namespace gui {

class help_message {
protected:
	std::string message = "";

public:
	void clear() {

		message.clear();
	}

	void add_line(const std::string& line) {

		if(!message.empty())
			message += "\n";

		message += line;
	}

	void add_bullet_point(const std::string& line) {
		
		add_line("  -\t" + line);
	}

	void show() const {

		cgv::gui::message(message);
	}

	void create_gui(cgv::gui::provider* p) const {
		
		cgv::gui::button_ptr btn = p->add_button("?", "w=0;font_style='bold'", "");
		
		int x = btn->get<int>("x");
		int y = btn->get<int>("y");

		cgv::base::group_ptr group = btn->get_parent() ? btn->get_parent()->get_group() : nullptr;
		
		if(group->get_nr_children() > 1) {
			cgv::base::base_ptr last_child = group->get_child(group->get_nr_children() - 2);

			int last_x = last_child->get<int>("x");
			int last_y = last_child->get<int>("y");
			int last_w = last_child->get<int>("w");
			int last_h = last_child->get<int>("h");

			x = last_x + (last_w > 200 ? last_w - 20 : 180);
			y = last_y;

		} else {
			x += 180;
		}

		btn->set("x", x);
		btn->set("y", y);
		btn->set("w", 20);

		connect_copy(btn->click, cgv::signal::rebind(this, &help_message::show));
	}
};

}
}
