#include "toggle.h"

#include <cgv_g2d/msdf_font.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

using namespace cgv::render;

namespace cg {
namespace g2d {

toggle::toggle(const std::string& label, cgv::g2d::irect rectangle) : widget(label, rectangle) {
	set_size(cgv::ivec2(40, 20));
}

bool toggle::set_value(bool v) {
	if(v == value)
		return false;

	value = v;
	update();
	return true;
}

bool toggle::handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 mouse_position) {
	cgv::gui::MouseAction action = e.get_action();

	if(e.get_button() == cgv::gui::MB_LEFT_BUTTON && action == cgv::gui::MA_PRESS) {
		if(rectangle.contains(mouse_position)) {
			if(set_value(!value))
				do_callback();
		}
	}
	
	return false;
}

void toggle::draw(context& ctx, cgv::g2d::canvas& cnvs, const styles& style) {
	cnvs.enable_shader(ctx, "rectangle");

	const int padding = 2;

	cgv::g2d::irect box = rectangle;
	cgv::g2d::irect knob = rectangle;
	knob.scale(-padding);
	knob.w() = std::min(knob.h(), knob.w() / 2);
	
	float box_radius = std::min(box.w(), box.h()) / 2.0f;
	float knob_radius = std::min(knob.w(), knob.h()) / 2.0f;

	box_radius = std::min(box_radius, knob_radius + padding);

	if(value)
		knob.translate(box.w() - knob.w() - 2 * padding, 0);

	cgv::g2d::shape2d_style box_style = style.rounded_box;
	box_style.border_radius = box_radius + 1;
	cnvs.set_style(ctx, box_style);
	cnvs.draw_shape(ctx, box, value ? style.highlight_color : style.background_color);

	box_style.border_radius = knob_radius;
	cnvs.set_style(ctx, box_style);
	cnvs.draw_shape(ctx, knob, style.control_color);
	cnvs.disable_current_shader(ctx);

	auto& font = cgv::g2d::ref_msdf_font_regular(ctx);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, label, style.text, cgv::ivec2(rectangle.x() - 5, rectangle.center().y()), TA_RIGHT);
}

}
}
