#pragma once

#include "control_base.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

class CGV_API input_control : public control_base {
public:
	enum class input_type {
		kString,
		kFloat,
		kInteger
	} type = input_type::kString;

private:
	bool focused = false;
	bool do_focus = false;
	float focus_position = 0.0f;
	size_t cursor_position = 0;

	std::string buffer;

	bool is_allowed(char c);

	void erase_at_cursor();

	void insert_at_cursor(char c);

public:
	using control_base::control_base;

	std::string get_value() const { return buffer; }

	bool set_value(const std::string& v);

	void update() override {}

	bool handle_key_event(cgv::gui::key_event& e) override;

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override;

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& style) override;
};

}
}

#include <cgv/config/lib_end.h>
