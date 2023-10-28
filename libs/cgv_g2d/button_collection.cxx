#pragma once

#include "button_collection.h"
#include <cgv_g2d/utils2d.h>

using namespace cgv::render;

namespace cgv {
namespace g2d {

button_collection::button_collection() {
	
	default_button_size = ivec2(120, 24);
}

void button_collection::destruct(context& ctx) {

	shaders.clear(ctx);

	ref_msdf_font_regular(ctx, -1);
	ref_msdf_gl_canvas_font_renderer(ctx, -1);

	buttons.clear();
}

void button_collection::clear() {

	buttons.clear();
	state_out_of_date = true;
}

bool button_collection::init(context& ctx) {

	bool success = true;

	shaders.add("rectangle", shaders::rectangle);
	success &= shaders.load_all(ctx, "button_collection::init()");

	msdf_font_regular& font = ref_msdf_font_regular(ctx, 1);
	ref_msdf_gl_canvas_font_renderer(ctx, 1);

	success &= font.is_initialized();

	if(success) {
		labels.set_msdf_font(&font);
		init_styles(ctx);
	}

	return success;
}

bool button_collection::handle(cgv::gui::event& e, const ivec2& viewport_size, const irect& container) {

	unsigned et = e.get_kind();

	if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		ivec2 mpos = get_local_mouse_pos(ivec2(me.get_x(), me.get_y()), viewport_size, container);

		if(me.get_button() == cgv::gui::MB_LEFT_BUTTON) {
			if(ma == cgv::gui::MouseAction::MA_PRESS) {
				for(size_t i = 0; i < buttons.size(); ++i) {
					const button& btn = buttons[i];

					if(btn.rect.is_inside(mpos)) {
						pressed_button_index = i;
						hovers_pressed_button = true;
						break;
					}
				}

				if(pressed_button_index != static_cast<size_t>(-1)) {
					return true;
				}
			} else if(ma == cgv::gui::MouseAction::MA_RELEASE) {
				if(pressed_button_index < buttons.size()) {
					const button& btn = buttons[pressed_button_index];

					if(btn.rect.is_inside(mpos)) {
						if(btn.callback) {
							btn.callback(btn.label);
						} else {
							if(default_callback)
								default_callback(btn.label);
						}
					}

					pressed_button_index = static_cast<size_t>(-1);
					hovers_pressed_button = false;

					return true;
				}
			}
		}

		if(me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if(pressed_button_index < buttons.size()) {
				hovers_pressed_button = buttons[pressed_button_index].rect.is_inside(mpos);
				return true;
			}
		}

		return false;
	}

	return false;
}

void button_collection::draw(context& ctx, cgv::g2d::canvas& cnvs) {

	if(style_out_of_date)
		init_styles(ctx);

	if(state_out_of_date)
		create_labels();

	auto& rect_prog = shaders.get("rectangle");
	cnvs.enable_shader(ctx, rect_prog);

	btn_style.apply(ctx, rect_prog);

	for(size_t i = 0; i < buttons.size(); ++i)
		cnvs.draw_shape(ctx, buttons[i].rect, button_press_color);

	for(size_t i = 0; i < buttons.size(); ++i) {
		irect rect = buttons[i].rect;
		rect.resize(0, -1);

		bool active = i == pressed_button_index && hovers_pressed_button;

		if(!active)
			rect.translate(0, 1);

		cnvs.draw_shape(ctx, rect, active ? button_press_color : button_color);
	}

	cnvs.disable_current_shader(ctx);

	if(hovers_pressed_button)
		move_label(-1);

	auto& font_renderer = cgv::g2d::ref_msdf_gl_canvas_font_renderer(ctx, 1);
	font_renderer.render(ctx, cnvs, labels, label_style);

	if(hovers_pressed_button)
		move_label(1);
}

void button_collection::add(const std::string& label, const ivec2& pos, std::function<void(const std::string&)> callback) {

	add(label, irect(pos, default_button_size), TextAlignment::TA_NONE, callback);
}

void button_collection::add(const std::string& label, const ivec2& pos, TextAlignment label_alignment, std::function<void(const std::string&)> callback) {

	add(label, irect(pos, default_button_size), label_alignment, callback);
}

void button_collection::add(const std::string& label, const irect& rect, std::function<void(const std::string&)> callback) {

	buttons.push_back({ rect, label, TextAlignment::TA_NONE, callback });
	state_out_of_date = true;
}

void button_collection::add(const std::string& label, const irect& rect, TextAlignment label_alignment, std::function<void(const std::string&)> callback) {

	buttons.push_back({ rect, label, label_alignment, callback });
	state_out_of_date = true;
}

void button_collection::init_styles(context& ctx) {

	auto& ti = cgv::gui::theme_info::instance();

	button_color = ti.control();
	button_press_color = ti.background();

	label_style.fill_color = ti.text();
	label_style.use_blending = true;
	label_style.font_size = 12.0f;
	label_style.enable_subpixel_rendering = true;

	btn_style.use_fill_color = false;
	btn_style.border_color = ti.border();
	btn_style.feather_width = 0.0f;
	btn_style.border_width = 0.0f;

	style_out_of_date = false;
}

void button_collection::handle_theme_change(const cgv::gui::theme_info& theme) {

	style_out_of_date = true;
}

void button_collection::create_labels() {

	labels.clear();

	for(auto& btn : buttons) {
		ivec2 pos = btn.rect.center();
		pos.y() += 1;

		if(btn.label_alignment & TextAlignment::TA_LEFT)
			pos.x() = 4;
		else if(btn.label_alignment & TextAlignment::TA_RIGHT)
			pos.x() = btn.rect.w() - 4;

		labels.add_text(btn.label, pos, btn.label_alignment);
	}

	state_out_of_date = false;
}

void button_collection::move_label(int offset) {

	if(pressed_button_index >= 0 && pressed_button_index < labels.size()) {
		ivec2 pos = labels.ref_texts()[pressed_button_index].position;
		pos.y() += offset;
		labels.set_position(static_cast<int>(pressed_button_index), pos);
	}
}

}
}
