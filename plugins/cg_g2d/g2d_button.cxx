#include "g2d_button.h"

#include <cgv_g2d/msdf_font.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

namespace cg {
namespace g2d {

bool g2d_button::handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 mouse_position) {
	cgv::gui::MouseAction action = e.get_action();

	if(e.get_button() == cgv::gui::MB_LEFT_BUTTON) {
		bool is_hit = rectangle.contains(mouse_position);

		if(action == cgv::gui::MouseAction::MA_PRESS && is_hit) {
			pressed = true;
			hovered = true;
			return true;
		}

		if(action == cgv::gui::MouseAction::MA_RELEASE && pressed) {
			if(is_hit)
				click(*this);

			pressed = false;
			hovered = false;
			return true;
		}
	}

	if(e.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
		if(pressed) {
			hovered = rectangle.contains(mouse_position);
			return true;
		}
	}

	return false;
}

void g2d_button::draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs, const styles& style) {
	cnvs.enable_shader(ctx, "rectangle");
	cnvs.set_style(ctx, style.flat_box);

	bool active = pressed && hovered;

	cnvs.draw_shape(ctx, rectangle, active ? style.shadow_color : style.background_color);

	cgv::g2d::irect top_rectangle = rectangle;
	top_rectangle.h() -= 1;

	if(!active)
		top_rectangle.translate(0, 1);

	cnvs.draw_shape(ctx, top_rectangle, active ? style.background_color : style.control_color);
	cnvs.disable_current_shader(ctx);

	cgv::ivec2 label_position = rectangle.center();

	

	// TODO: move to method in control_base
	if(label_alignment & cgv::render::TA_LEFT)
		label_position.x() = rectangle.x() + 5;
	else if(label_alignment & cgv::render::TA_RIGHT)
		label_position.x() = rectangle.x1() - 5;

	if(label_alignment & cgv::render::TA_TOP)
		label_position.y() = rectangle.y1() - 1;
	else if(label_alignment & cgv::render::TA_BOTTOM)
		label_position.y() = rectangle.y() + 1;



	
	if(active)
		label_position.y() -= 1;

	auto& font = cgv::g2d::ref_msdf_font_regular(ctx);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, label, style.text, label_position, label_alignment);
}

}
}
