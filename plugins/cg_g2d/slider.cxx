#include "slider.h"

#include <cgv_g2d/msdf_font.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

using namespace cgv::render;

namespace cg {
namespace g2d {

void slider::update_value() {
	cgv::dvec2 in_range(rectangle.x(), rectangle.x1() - handle.w());
	double next_value = cgv::math::map(static_cast<double>(handle.x()), in_range[0], in_range[1], get_range()[0], get_range()[1]);
	next_value = cgv::math::clamp(next_value, get_range()[0], get_range()[1]);

	handle_value_change(next_value);
}

void slider::update_handle() {
	cgv::dvec2 out_range(rectangle.x(), rectangle.x1() - handle.w());
	double position = static_cast<float>(cgv::math::map(get_value(), get_range()[0], get_range()[1], out_range[0], out_range[1]));
	handle_view_position = static_cast<int>(cgv::math::clamp(position, out_range[0], out_range[1]));
}

slider::slider(const std::string& label, cgv::g2d::irect rectangle) : valuator(label, rectangle) {
	handle = {
		static_cast<cgv::vec2>(rectangle.position),
		cgv::vec2(12.0f, static_cast<float>(rectangle.h()))
	};
	draggables.add(&handle);
	draggables.set_constraint(rectangle);
	draggables.set_drag_callback(std::bind(&slider::update_value, this));

	handle_view_position = rectangle.x();
}

bool slider::handle_mouse_event(cgv::gui::mouse_event& e, cgv::ivec2 mouse_position) {
	//if(!rectangle.contains(mouse_position))
	//	return false;

	cgv::gui::MouseAction action = e.get_action();

	if(e.get_button() == cgv::gui::MB_LEFT_BUTTON && action == cgv::gui::MA_PRESS) {
		if(rectangle.contains(mouse_position) && !handle.contains(mouse_position)) {
			handle.position = mouse_position.x() - 0.5f * handle.w();
			handle.apply_constraint(rectangle);

			update_value();
		}
	}

	if(action == cgv::gui::MA_WHEEL) {
		if(rectangle.contains(mouse_position)) {
			double speed = (e.get_modifiers() & cgv::gui::EM_SHIFT) ? 4.0 : 1.0;
			
			double next_value = get_value();
			next_value += e.get_dy() * speed * get_step();
			next_value = cgv::math::clamp(next_value, get_range()[0], get_range()[1]);

			handle_value_change(next_value);
			return true;
		}
	}

	if(draggables.handle_mouse_event(e, mouse_position))
		return true;

	return false;
}

void slider::draw(context& ctx, cgv::g2d::canvas& cnvs, const styles& style) {
	cnvs.enable_shader(ctx, "rectangle");
	cnvs.set_style(ctx, style.flat_box);

	cgv::g2d::irect track = rectangle;
	track.scale(0, -5);
	cnvs.draw_shape(ctx, track, style.background_color);

	cgv::g2d::rect handle_rectangle = handle;
	handle_rectangle.x() = handle_view_position;
	cnvs.draw_shape(ctx, handle_rectangle, style.control_color);
	cnvs.disable_current_shader(ctx);

	auto& font = cgv::g2d::ref_msdf_font_regular(ctx);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, label, style.text, cgv::ivec2(rectangle.x() - 5, rectangle.center().y()), TA_RIGHT);
}

}
}
