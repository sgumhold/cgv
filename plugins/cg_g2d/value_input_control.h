#pragma once

#include "input_control.h"
#include "value_control.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

class CGV_API value_input_control : public value_control {
public:
	input_control input;

	value_input_control(const std::string& label, cgv::g2d::irect rectangle);

	bool set_value(double v);
	
	void update() override {}

	bool handle_key_event(cgv::gui::key_event& e) override {
		return input.handle_key_event(e);
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		return input.handle_mouse_event(e, mouse_position);
	}

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& styles) override {
		input.draw(ctx, cnvs, styles);
	}
};

}
}

#include <cgv/config/lib_end.h>
