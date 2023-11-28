#include "slider_control.h"

#include <cgv_g2d/msdf_font.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>

using namespace cgv::render;

namespace cg {
namespace g2d {

// TODO: move to math lib and add cgv::math::clamp with arguments (T, fvec<T, 2>)
template<typename T>
T map(T value, cgv::math::fvec<T, 2>& in_range, cgv::math::fvec<T, 2>& out_range) {
	return out_range[0] + (out_range[1] - out_range[0]) * ((value - in_range[0]) / (in_range[1] - in_range[0]));
}

void slider_control::update_value() {
	//auto& range = get_range();
	//double t = (handle.x() - rectangle.x()) / (rectangle.w() - handle.w());
	//double next_value = cgv::math::lerp(range[0], range[1], t);
	//next_value = cgv::math::clamp(next_value, range[0], range[1]);

	dvec2 in_range(rectangle.x(), rectangle.x1() - handle.w());
	double next_value = map(static_cast<double>(handle.x()), in_range, get_range());
	next_value = cgv::math::clamp(next_value, get_range()[0], get_range()[1]);

	handle_value_change(next_value);
}

void slider_control::update_handle() {
	//auto& range = get_range();
	//double t = cgv::math::clamp(get_value(), range[0], range[1]);
	//t = (t - range[0]) / (range[1] - range[0]);
	//handle.x() = rectangle.x() + static_cast<int>(t * (rectangle.w() - handle.w()) + 0.5f);

	dvec2 out_range(rectangle.x(), rectangle.x1() - handle.w());
	double position = static_cast<float>(map(get_value(), get_range(), out_range));
	handle.x() = static_cast<float>(cgv::math::clamp(position, out_range[0], out_range[1]));// +0.5f;
}

slider_control::slider_control(const std::string& label, cgv::g2d::irect rectangle) : value_control(label, rectangle) {
	handle.position = static_cast<vec2>(rectangle.position);
	handle.size = vec2(12.0f, static_cast<float>(rectangle.h()));
	handle_draggable.add(&handle);
	handle_draggable.set_constraint(rectangle);
	handle_draggable.set_drag_callback(std::bind(&slider_control::update_value, this));
}

bool slider_control::handle_mouse_event(cgv::gui::mouse_event& e, ivec2 mouse_position) {
	//if(!rectangle.is_inside(mouse_position))
	//	return false;

	cgv::gui::MouseAction action = e.get_action();

	if(e.get_button() == cgv::gui::MB_LEFT_BUTTON && action == cgv::gui::MA_PRESS) {
		if(rectangle.is_inside(mouse_position) && !handle.is_inside(mouse_position)) {
			handle.position = mouse_position.x() - 0.5f * handle.w();
			handle.apply_constraint(rectangle);

			update_value();
		}
	}

	if(action == cgv::gui::MA_WHEEL) {
		if(rectangle.is_inside(mouse_position)) {
			double speed = (e.get_modifiers() & cgv::gui::EM_SHIFT) ? 4.0 : 1.0;
			
			double next_value = get_value();
			next_value += e.get_dy() * speed * get_step();
			next_value = cgv::math::clamp(next_value, get_range()[0], get_range()[1]);

			handle_value_change(next_value);
			return true;
		}
	}

	if(handle_draggable.handle_mouse_event(e, mouse_position))
		return true;

	return false;
}

void slider_control::draw(context& ctx, cgv::g2d::canvas& cnvs, const styles& style) {
	cnvs.enable_shader(ctx, "rectangle");
	cnvs.set_style(ctx, style.colored_box);

	cgv::g2d::irect track = rectangle;
	track.scale(0, -5);
	cnvs.draw_shape(ctx, track, style.background_color);

	cnvs.draw_shape(ctx, handle, style.control_color);
	cnvs.disable_current_shader(ctx);

	auto& font = cgv::g2d::ref_msdf_font_regular(ctx);
	cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, label, style.text, ivec2(rectangle.x() - 5, rectangle.center().y()), TA_RIGHT);
}

}
}
