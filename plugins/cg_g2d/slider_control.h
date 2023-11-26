#pragma once

#include "value_control.h"

#include <cgv_g2d/draggable.h>
#include <cgv_g2d/draggable_collection.h>

#include "lib_begin.h"

namespace cg {
namespace g2d {

class CGV_API slider_control : public value_control {
private:
	cgv::g2d::draggable handle;
	cgv::g2d::draggable_collection<cgv::g2d::draggable*> handle_draggable;

	void update_value();

	void update_handle();

public:
	slider_control(const std::string& label, cgv::g2d::irect rectangle);

	void update() override { update_handle(); }

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override;

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& style) override;
};

}
}

#include <cgv/config/lib_end.h>
