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

public:
	control_base(const std::string& label, cgv::g2d::irect rectangle) : label(label), rectangle(rectangle) {}

	virtual ~control_base() {}

	void set_label(const std::string& label) {
		this->label = label;
	}

	virtual void update() = 0;

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
