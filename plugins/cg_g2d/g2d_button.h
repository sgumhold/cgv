#pragma once

#include <cgv/gui/button.h>

#include "widget.h"

#include "lib_begin.h"

namespace cg {
namespace g2d {

/// Implements the button gui element with a g2d button.
class CGV_API g2d_button : public cgv::gui::button, public widget {
private:
	/// whether the button was pressed on left mouse button down
	bool pressed = false;
	/// whether the mouse pointer is currently inside the button rectangle
	bool hovered = false;
	
public:
	/// construct from label and rectangle
	g2d_button(const std::string& label, cgv::g2d::irect rectangle) : cgv::gui::button(label), widget(label, rectangle) {}
	/// destruct g2d button
	~g2d_button() {}
	/// return "g2d_button"
	std::string get_type_name() const { return "g2d_button"; }
	/// handle mouse events
	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 mouse_position) override;
	/// draw the button with its label
	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& style) override;
};

/// ref counted pointer to g2d_button
typedef cgv::data::ref_ptr<g2d_button> g2d_button_ptr;

#if _MSC_VER >= 1400
CGV_TEMPLATE template class CGV_API cgv::data::ref_ptr<g2d_button>;
#endif

}
}

#include <cgv/config/lib_end.h>
