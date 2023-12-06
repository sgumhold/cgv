#pragma once

#include "input.h"
#include "valuator.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

class CGV_API value_input : public valuator {
private:
	void update_input();

public:
	input input_widget;

	value_input(const std::string& label, cgv::g2d::irect rectangle);

	bool set_value(double v);
	
	bool handle_key_event(cgv::gui::key_event& e) override {
		if(input_widget.is_focused() && (e.get_action() == cgv::gui::KA_PRESS || e.get_action() == cgv::gui::KA_REPEAT)) {
			switch(e.get_key()) {
			case cgv::gui::KEY_Up:
				double next_value = get_value() + get_step()
				handle_value_change(next_value);
				input_widget.focus();
				return true;
			case cgv::gui::KEY_Down:
				handle_value_change(get_value() - get_step());
				input_widget.focus();
				return true;
			default: break;
			}
		}

		return input_widget.handle_key_event(e);
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		return input_widget.handle_mouse_event(e, mouse_position);
	}

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& styles) override {
		input_widget.draw(ctx, cnvs, styles);
	}
};

}
}

#include <cgv/config/lib_end.h>
