#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv/render/render_types.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>
#include <cgv_g2d/rect.h>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

class button_collection : public cgv::render::render_types, cgv::gui::theme_observer {
public:
	text2d_style label_style;
	msdf_text_geometry labels;

	shape2d_style btn_style;
	rgba button_color;
	rgba button_press_color;

	bool update_styles = false;

	cgv::render::shader_library shaders;

	struct button {
		std::string label;
		irect rect;
		std::function<void(const std::string&)> callback;
	};

	std::vector<button> buttons;
	size_t pressed_button_index = static_cast<size_t>(-1);
	bool hovers_pressed_button = false;

	bool state_out_of_date = true;

	button_collection() {}

	void destruct(cgv::render::context& ctx) {

		shaders.clear(ctx);

		ref_msdf_font_light(ctx, -1);
		ref_msdf_gl_canvas_font_renderer(ctx, -1);

		buttons.clear();
	}

	void clear() {
		buttons.clear();
		state_out_of_date = true;
	}

	bool init(cgv::render::context& ctx) {

		bool success = true;

		shaders.add("rectangle", shaders::rectangle);
		success &= shaders.load_all(ctx, "button_collection::init()");

		msdf_font_light& font = ref_msdf_font_light(ctx, 1);
		ref_msdf_gl_canvas_font_renderer(ctx, 1);

		success &= font.is_initialized();

		if(success) {
			labels.set_msdf_font(&font);
			init_styles(ctx);
		}

		return success;
	}

	bool handle(cgv::gui::event& e, const ivec2& viewport_size, const irect& container = irect()) {
		unsigned et = e.get_kind();

		if(et == cgv::gui::EID_MOUSE) {
			cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
			cgv::gui::MouseAction ma = me.get_action();

			ivec2 mpos(me.get_x(), me.get_y());
			mpos.y() = viewport_size.y() - mpos.y() - 1;
			mpos -= container.pos();

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
							if(btn.callback)
								btn.callback(btn.label);
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

	void move_label(int offset) {
		if(pressed_button_index >= 0 && pressed_button_index < labels.size()) {
			ivec2 pos = labels.ref_texts()[pressed_button_index].position;
			pos.y() += offset;
			labels.set_position(static_cast<int>(pressed_button_index), pos);
		}
	}

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs) {

		if(update_styles) {
			update_styles = false;
			init_styles(ctx);
		}

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

	void add(const std::string& label, const irect& rect, std::function<void(const std::string&)> callback) {

		buttons.push_back({ label, rect, callback });
		state_out_of_date = true;
	}

	void add(const std::string& label, const ivec2& pos, const ivec2& size, std::function<void(const std::string&)> callback) {

		buttons.push_back({ label, irect(pos, size), callback });
		state_out_of_date = true;
	}

	void create_labels() {

		labels.clear();

		for(auto& btn : buttons) {
			ivec2 pos = btn.rect.center();
			pos.y() += 2;
			labels.add_text(btn.label, pos, cgv::render::TextAlignment::TA_NONE);
		}

		state_out_of_date = false;
	}

	void handle_theme_change(const cgv::gui::theme_info& theme) override {
		update_styles = true;
	}

	void init_styles(cgv::render::context& ctx) {

		auto& ti = cgv::gui::theme_info::instance();

		button_color = rgba(ti.control(), 1.0f);
		button_press_color = rgba(ti.background(), 1.0f);

		label_style.fill_color = rgba(ti.text(), 1.0f);
		label_style.use_blending = true;
		label_style.font_size = 12.0f;
		label_style.clear_type_mode = ti.is_dark() ?
			text2d_style::ClearTypeMode::CT_WHITE_ON_BLACK :
			text2d_style::ClearTypeMode::CT_BLACK_ON_WHITE;

		btn_style.use_fill_color = false;
		btn_style.border_color = rgba(ti.border(), 1.0f);
		btn_style.feather_width = 0.0f;
		btn_style.border_width = 0.0f;
	}
};

}
}

#include <cgv/config/lib_end.h>
