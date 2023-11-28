#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/render/render_types.h>
#include <cgv_g2d/canvas.h>
#include <cgv_g2d/rect.h>

#include "styles.h"

namespace cg {
namespace g2d {

class control_base {
private:
	typedef std::function<void(control_base*, void*)> callback_t;
	callback_t _callback;
	void* _user_data = nullptr;

protected:
	std::string label;
	cgv::g2d::irect rectangle;

	cgv::render::TextAlignment label_alignment = cgv::render::TextAlignment::TA_NONE;

public:
	control_base(const std::string& label, cgv::g2d::irect rectangle) : label(label), rectangle(rectangle) {}

	virtual ~control_base() {}

	const std::string& get_label(const std::string& label) const { return label; }

	void set_label(const std::string& label) { this->label = label; }

	cgv::render::ivec2 get_position() const { return rectangle.position; }

	void set_position(cgv::render::ivec2 position) { rectangle.position = position; }

	cgv::render::ivec2 get_size() const { return rectangle.size; }

	void set_size(cgv::render::ivec2 size) { rectangle.size = size; }

	cgv::render::TextAlignment get_label_alignment() const { return label_alignment; }

	void set_label_alignment(cgv::render::TextAlignment alignment) { label_alignment = alignment; }

	virtual void update() {}

	void callback(callback_t c, void* p) {
		_callback = c;
		_user_data = p;
	}

	void do_callback() {
		if(_callback)
			_callback(this, _user_data);
	}

	virtual bool handle_key_event(cgv::gui::key_event& e) { return false; }
	virtual bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) { return false; }

	virtual void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& style) {}
};

}
}
