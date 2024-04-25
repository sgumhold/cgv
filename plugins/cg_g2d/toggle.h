#pragma once

#include "widget.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

class CGV_API toggle : public widget {
private:
	bool value = false;

public:
	toggle(const std::string& label, cgv::g2d::irect rectangle);

	bool get_value() const { return value; }

	bool set_value(bool v);

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 mouse_position) override;

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& style) override;
};

}
}

#include <cgv/config/lib_end.h>
