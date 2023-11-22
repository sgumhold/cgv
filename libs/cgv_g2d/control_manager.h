#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/theme_info.h>
#include <cgv/render/render_types.h>
#include <cgv/signal/rebind.h>
#include <cgv/signal/signal.h>
#include <cgv_g2d/draggable_collection.h>
#include <cgv_g2d/msdf_gl_canvas_font_renderer.h>
#include <cgv_g2d/rect.h>

#include "lib_begin.h"

namespace cgv {
namespace g2d {

struct control_styles {
	shape2d_style control_box;
	shape2d_style colored_box;
	text2d_style text;

	cgv::render::rgb control_color;
	cgv::render::rgb background_color;
	cgv::render::rgb shadow_color;
};

class CGV_API control_base {
protected:
	std::string label;
	irect rectangle;
	
public:
	control_base(irect rectangle) : rectangle(rectangle) {}

	virtual ~control_base() {}

	virtual bool handle_key_event(cgv::gui::key_event& e) { return false; }
	virtual bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) { return false; }

	virtual void draw(cgv::render::context& ctx, canvas& cnvs, const control_styles& styles) {}

	void set_label(const std::string& label) {
		this->label = label;
	}
};

class CGV_API button_control : public control_base {
private:
	bool pressed = false;
	bool hovered = false;

public:
	using control_base::control_base;

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		cgv::gui::MouseAction action = e.get_action();

		if(e.get_button() == cgv::gui::MB_LEFT_BUTTON) {
			bool is_hit = rectangle.is_inside(mouse_position);

			if(action == cgv::gui::MouseAction::MA_PRESS && is_hit) {
				pressed = true;
				hovered = true;
				return true;
			}
			
			if(action == cgv::gui::MouseAction::MA_RELEASE && pressed) {
				if(is_hit)
					on_click(*this);
				
				pressed = false;
				hovered = false;
				return true;
			}
		}

		if(e.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if(pressed) {
				hovered = rectangle.is_inside(mouse_position);
				return true;
			}
		}

		return false;
	}

	void draw(cgv::render::context& ctx, canvas & cnvs, const control_styles& styles) override {
		cnvs.enable_shader(ctx, "rectangle");
		cnvs.set_style(ctx, styles.colored_box);

		bool active = pressed && hovered;

		cnvs.draw_shape(ctx, rectangle, active ? styles.shadow_color : styles.background_color);

		irect top_rectangle = rectangle;
		top_rectangle.resize(0, -1);

		if(!active)
			top_rectangle.translate(0, 1);

		cnvs.draw_shape(ctx, top_rectangle, active ? styles.background_color : styles.control_color);
		cnvs.disable_current_shader(ctx);

		cgv::render::ivec2 label_position = rectangle.center();

		if(active)
			label_position.y() -= 1;

		auto& font = ref_msdf_font_regular(ctx);
		ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, label, styles.text, label_position);
	}

	cgv::signal::signal<button_control&> on_click;
};

class CGV_API text_input_control : public control_base {
private:
	bool focused = false;
	bool do_focus = false;
	float focus_position;
	size_t caret_position = 0;

	std::string* value = nullptr;

public:
	using control_base::control_base;

	bool handle_key_event(cgv::gui::key_event& e) override {
		if(!value)
			return false;

		cgv::gui::KeyAction action = e.get_action();

		if(focused && (action == cgv::gui::KA_PRESS || action == cgv::gui::KA_REPEAT)) {
			switch(e.get_key()) {
			case cgv::gui::KEY_Home:
				caret_position = 0;
				return true;
			case cgv::gui::KEY_End:
				caret_position = value->length();
				return true;
			case cgv::gui::KEY_Left:
				if(caret_position > 0)
					--caret_position;
				return true;
			case cgv::gui::KEY_Right:
				if(caret_position < value->length())
					++caret_position;
				return true;
			case cgv::gui::KEY_Back_Space:
				if(caret_position > 0 && caret_position <= value->length()) {
					--caret_position;
					value->erase(caret_position, 1);
					on_change(*this);
				}
				return true;
			case cgv::gui::KEY_Delete:
				if(caret_position < value->length()) {
					value->erase(caret_position, 1);
					on_change(*this);
				}
				return true;
			//case cgv::gui::KEY_Enter:
				//	editing_active = false;
				//
				//	return true;
			case cgv::gui::KEY_Space:
				value->insert(caret_position, 1, ' ');
				++caret_position;
				on_change(*this);
				return true;
			case '\\':
				// capture espace sequences, e.g, when pressing '^'
				return true;
			default:
				if(e.get_key() < 256) {
					//std::cout << "key = " << ke.get_key() << ", char = '" << ke.get_char() << "', char_num = " << static_cast<int>(ke.get_char()) << std::endl;
					unsigned char c = e.get_char();
					if(std::isprint(c)) {
						value->insert(caret_position, 1, c);
						++caret_position;
						on_change(*this);
					}
					return true;
				}
				break;
			}
		}

		return false;
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		if(!value)
			return false;

		cgv::gui::MouseAction action = e.get_action();

		if(e.get_button() == cgv::gui::MB_LEFT_BUTTON) {
			if(action == cgv::gui::MouseAction::MA_PRESS) {
				if(rectangle.is_inside(mouse_position)) {
					focused = true;
					do_focus = true;
					focus_position = mouse_position.x();
					caret_position = value->length();
					return true;
				} else if(focused) {
					// TODO: do we even need to deactivate the input? FLTK only removes the caret when the mouse leaves the gui
					focused = false;
					caret_position = 0;
					// TOOD: should issue redraw but not cancel event
					//return true;
				}
			}
		}

		return false;
	}

	void draw(cgv::render::context& ctx, canvas& cnvs, const control_styles& styles) override {

		// TODO: place in seperate method?
		auto& font = ref_msdf_font_regular(ctx);

		if(do_focus && value) {
			do_focus = false;
			caret_position = 0;

			float position = rectangle.x() + 5.0f;
			float closest_distance = std::abs(position - focus_position);
			float last_distance = closest_distance;

			for(size_t i = 0; i < value->length(); ++i) {
				const auto& glyph = font.get_glyph_info(static_cast<unsigned char>(value->at(i)));

				position += glyph.advance * styles.text.font_size;
				float distance = std::abs(position - focus_position);

				if(distance < closest_distance) {
					closest_distance = distance;
					caret_position = i + 1;
				} else {
					break;
				}

				last_distance = distance;
			}
		}





		cnvs.enable_shader(ctx, "rectangle");
		cnvs.set_style(ctx, styles.control_box);
		cnvs.draw_shape(ctx, rectangle);
		cnvs.disable_current_shader(ctx);

		cgv::render::ivec2 position(rectangle.x(), rectangle.center().y());
		
		ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, label, styles.text, position - cgv::render::ivec2(5, 0), cgv::render::TA_RIGHT);
		if(value) {
			ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, *value, styles.text, position + cgv::render::ivec2(5, 0), cgv::render::TA_LEFT);

			if(focused) {
				cnvs.enable_shader(ctx, "rectangle");
				cnvs.set_style(ctx, styles.colored_box);

				cgv::render::vec2 text_size = styles.text.font_size * cgv::render::vec2(font.compute_length(*value, caret_position), 1.0f);

				cgv::render::vec2 position = rectangle.position;
				position.x() += text_size.x() + 5.0f;
				position.x() = std::floor(position.x()) + 0.5f;
				position.y() += 2.0f;

				cnvs.draw_shape(ctx, position, cgv::render::vec2(1.0f, 16.0f), styles.text.fill_color);
				cnvs.disable_current_shader(ctx);
			}
		}
	}

	void set_value_ptr(std::string* ptr) {
		value = ptr;
	}

	cgv::signal::signal<text_input_control&> on_change;
};


// TODO: rename to slider_control or slider_input (same for text_input_control)?
class CGV_API slider_input_control : public control_base {
private:
	cgv::g2d::draggable handle;
	cgv::g2d::draggable_collection<cgv::g2d::draggable*> handle_draggable;

	float* value = nullptr;
	cgv::render::vec2 range = cgv::render::vec2(0.0f, 1.0f);

	void update_value() {
		if(value) {
			float t = (handle.x() - rectangle.x()) / (rectangle.w() - handle.w());
			float next_value = cgv::math::lerp(range[0], range[1], t);
			next_value = cgv::math::clamp(next_value, range[0], range[1]);

			// TODO: implement special case for integers
			if(std::abs(*value - next_value) >= std::numeric_limits<float>::epsilon()) {
				*value = next_value;
				on_change(*this);
			}
		}
	}

	void update_handle() {
		if(value) {
			float t = cgv::math::clamp(*value, range[0], range[1]);
			t = (t - range[0]) / (range[1] - range[0]);
			handle.x() += static_cast<int>(t * (rectangle.w() - 10) + 0.5f);
		}
	}

public:
	slider_input_control(irect rectangle) : control_base(rectangle) {
		handle.position = static_cast<cgv::render::vec2>(rectangle.position);
		handle.size = cgv::render::vec2(12.0f, static_cast<float>(rectangle.h()));
		handle_draggable.add(&handle);
		handle_draggable.set_constraint(rectangle);
		handle_draggable.set_drag_callback(std::bind(&slider_input_control::update_value, this));
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		if(!value)
			return false;

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
			float speed = (e.get_modifiers() & cgv::gui::EM_SHIFT) ? 4.0f : 1.0f;

			handle.x() += speed * e.get_dy();
			handle.apply_constraint(rectangle);

			update_value();
			return true;
		}

		if(handle_draggable.handle_mouse_event(e, mouse_position))
			return true;

		return false;
	}

	void draw(cgv::render::context& ctx, canvas& cnvs, const control_styles& styles) override {
		cnvs.enable_shader(ctx, "rectangle");
		cnvs.set_style(ctx, styles.colored_box);

		irect track = rectangle;
		track.scale(0, -5);
		cnvs.draw_shape(ctx, track, styles.background_color);

		cnvs.draw_shape(ctx, handle, styles.control_color);
		cnvs.disable_current_shader(ctx);

		auto& font = ref_msdf_font_regular(ctx);
		ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, label, styles.text, cgv::render::ivec2(rectangle.x() - 5, rectangle.center().y()), cgv::render::TA_RIGHT);
	}

	void set_value_ptr(float* ptr) {
		value = ptr;

		update_handle();
	}

	cgv::signal::signal<slider_input_control&> on_change;
};












class CGV_API control_manager : cgv::gui::theme_observer {
protected:
	cgv::render::shader_library shaders;

	control_styles styles;
	cgv::render::ivec2 default_control_size = cgv::render::ivec2(200, 20);

	std::vector<std::shared_ptr<control_base>> controls;

	std::function<void(void*)> default_on_change_callback;

	void init_styles();

	void handle_theme_change(const cgv::gui::theme_info& theme) override;

	template<typename T>
	std::shared_ptr<T> add_control(const std::string& label, cgv::render::ivec2 position) {
		auto ptr = std::make_shared<T>(irect(position, default_control_size));
		ptr->set_label(label);
		controls.push_back(ptr);
		return ptr;
	}

public:
	control_manager();

	bool init(cgv::render::context& ctx);

	void destruct(cgv::render::context& ctx);

	void clear();

	bool handle(cgv::gui::event& e, const cgv::render::ivec2& viewport_size, const irect& container = irect());

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs);



	

	std::shared_ptr<button_control> add_button(const std::string& label, cgv::render::ivec2 position) {
		return add_control<button_control>(label, position);
	}

	std::shared_ptr<text_input_control> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, std::string& value) {
		auto ptr = add_control<text_input_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}

	std::shared_ptr<slider_input_control> add_slider_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, float& value) {
		auto ptr = add_control<slider_input_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}
};

}
}

#include <cgv/config/lib_end.h>
