#pragma once

#include <cgv/gui/dialog.h>
#include <cgv/gui/provider.h>

namespace cgv {
namespace app {

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
		
		connect_copy(p->add_button("?", "w=20;font_style='bold'")->click, cgv::signal::rebind(this, &help_message::show));
	}
};

}
}
