#pragma once

#include "widget.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

class CGV_API input : public widget {
public:
	enum class Type {
		kString,
		kFloat,
		kInteger
	} type = Type::kString;

private:
	bool focused = false;
	bool do_focus = false;
	float focus_position = 0.0f;
	size_t cursor_position = 0;

	std::string text;

	bool is_allowed(char c);

	void erase_at_cursor();

	void insert_at_cursor(char c);

public:
	using widget::widget;

	Type get_type() const { return type; }

	void set_type(Type type) { this->type = type; }

	std::string get_value() const { return text; }

	bool set_value(const std::string& v);

	size_t get_cursor_position() const { return cursor_position; }

	bool is_focused() const { return focused; }

	void focus() { focused = true; }

	void update() override {}

	bool handle_key_event(cgv::gui::key_event& e) override;

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 mouse_position) override;

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& style) override;
};

}
}

#include <cgv/config/lib_end.h>
