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

	void set_label(const std::string& label) {
		this->label = label;
	}

	virtual const void* get_value_ptr() const = 0;

	virtual void update() = 0;

	virtual bool handle_key_event(cgv::gui::key_event& e) { return false; }
	virtual bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) { return false; }

	virtual void draw(cgv::render::context& ctx, canvas& cnvs, const control_styles& styles) {}
};

class CGV_API button_control : public control_base {
private:
	bool pressed = false;
	bool hovered = false;

public:
	using control_base::control_base;

	const void* get_value_ptr() const override {
		return nullptr;
	};

	void update() override {}

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


// TODO: Set value onyl once after construction and then allow arbitrary inputs. Validate each input before setting the value_ptr.
template<typename T>
class CGV_API text_input_control : public control_base {
private:
	bool focused = false;
	bool do_focus = false;
	float focus_position;
	size_t caret_position = 0;

	void erase_character() {
		text.erase(caret_position, 1);
		update_value();
		on_change(*this);
	}

	void insert_character(char c) {
		text.insert(caret_position, 1, c);
		++caret_position;
		update_value();
		on_change(*this);
	}

protected:
	std::string text = "";
	T* value_ptr = nullptr;

	virtual void update_value() = 0;

public:
	using control_base::control_base;

	const void* get_value_ptr() const override {
		return value_ptr;
	};

	/*
	void set_value_ptr(std::string* ptr) {
		value_ptr = ptr;
		if(ptr)
			text = *ptr;
	}
	*/

	virtual void set_value_ptr(T* ptr) = 0;
	//void set_value_ptr(std::string* ptr) {
	//	value_ptr = ptr;
	//	update();
	//}

	virtual void update() = 0;

	bool handle_key_event(cgv::gui::key_event& e) override {
		//if(!value_ptr)
		//	return false;

		cgv::gui::KeyAction action = e.get_action();

		if(focused && (action == cgv::gui::KA_PRESS || action == cgv::gui::KA_REPEAT)) {
			switch(e.get_key()) {
			case cgv::gui::KEY_Home:
				caret_position = 0;
				return true;
			case cgv::gui::KEY_End:
				caret_position = text.length();
				return true;
			case cgv::gui::KEY_Left:
				if(caret_position > 0)
					--caret_position;
				return true;
			case cgv::gui::KEY_Right:
				if(caret_position < text.length())
					++caret_position;
				return true;
			case cgv::gui::KEY_Back_Space:
				if(caret_position > 0 && caret_position <= text.length()) {
					--caret_position;
					erase_character();
				}
				return true;
			case cgv::gui::KEY_Delete:
				if(caret_position < text.length())
					erase_character();
				return true;
			case cgv::gui::KEY_Space:
				insert_character(' ');
				return true;
			case '\\':
				// capture escape sequences, e.g, when pressing '^'
				return true;
			default:
				if(e.get_key() < 256) {
					//std::cout << "key = " << ke.get_key() << ", char = '" << ke.get_char() << "', char_num = " << static_cast<int>(ke.get_char()) << std::endl;
					unsigned char c = e.get_char();
					if(std::isprint(c))
						insert_character(c);
					return true;
				}
				break;
			}
		}

		return false;
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		//if(!value_ptr)
		//	return false;

		cgv::gui::MouseAction action = e.get_action();

		if(e.get_button() == cgv::gui::MB_LEFT_BUTTON) {
			if(action == cgv::gui::MouseAction::MA_PRESS) {
				if(rectangle.is_inside(mouse_position)) {
					focused = true;
					do_focus = true;
					focus_position = mouse_position.x();
					caret_position = text.length();
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
		auto& font = ref_msdf_font_regular(ctx);

		if(do_focus) {// && value_ptr) {
			do_focus = false;
			caret_position = 0;

			float position = rectangle.x() + 5.0f;
			float closest_distance = std::abs(position - focus_position);
			float last_distance = closest_distance;

			for(size_t i = 0; i < text.length(); ++i) {
				const auto& glyph = font.get_glyph_info(static_cast<unsigned char>(text[i]));

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
		//if(value_ptr) {
			ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, text, styles.text, position + cgv::render::ivec2(5, 0), cgv::render::TA_LEFT);

			if(focused) {
				cnvs.enable_shader(ctx, "rectangle");
				cnvs.set_style(ctx, styles.colored_box);

				cgv::render::vec2 text_size = styles.text.font_size * cgv::render::vec2(font.compute_length(text, caret_position), 1.0f);

				cgv::render::vec2 position = rectangle.position;
				position.x() += text_size.x() + 5.0f;
				position.x() = std::floor(position.x()) + 0.5f;
				position.y() += 2.0f;

				cnvs.draw_shape(ctx, position, cgv::render::vec2(1.0f, 16.0f), styles.text.fill_color);
				cnvs.disable_current_shader(ctx);
			}
		//}
	}

	cgv::signal::signal<text_input_control&> on_change;
};

class CGV_API text_input_control_string : public text_input_control<std::string> {
private:
	void update_value() override {
		if(value_ptr)
			*value_ptr = text;
	}

public:
	using text_input_control<std::string>::text_input_control;

	void set_value_ptr(std::string* ptr) override {
		value_ptr = ptr;
		update();
		//if(ptr)
		//	text = *ptr;
	}

	void update() override {
		if(value_ptr)
			text = *value_ptr;
	}
};

class CGV_API text_input_control_float : public text_input_control<float> {
private:
	void update_value() override {
		if(value_ptr)
			// TODO: exception handling
			*value_ptr = std::stof(text);
	}

public:
	using text_input_control<float>::text_input_control;

	void set_value_ptr(float* ptr) override {
		value_ptr = ptr;
		update();
		//if(ptr)
		//	text = std::to_string(*ptr);
	}

	void update() override {
		if(value_ptr)
			text = std::to_string(*value_ptr);
	}
};





// TODO: rename to slider_control or slider_input (same for text_input_control)?
//template<typename T>
class CGV_API slider_input_control : public control_base {
private:
	cgv::g2d::draggable handle;
	cgv::g2d::draggable_collection<cgv::g2d::draggable*> handle_draggable;

	float* value_ptr = nullptr;
	cgv::render::vec2 range = cgv::render::vec2(0.0f, 1.0f);

	//T* value = nullptr;
	//cgv::math::fvec<T, 2U> range = cgv::math::fvec<T, 2U>(std::numeric_limits<T>::min(), std::numeric_limits<T::max());

	void update_value() {
		if(value_ptr) {
			float t = (handle.x() - rectangle.x()) / (rectangle.w() - handle.w());
			float next_value = cgv::math::lerp(range[0], range[1], t);
			next_value = cgv::math::clamp(next_value, range[0], range[1]);

			// TODO: implement special case for integers
			if(std::abs(*value_ptr - next_value) >= std::numeric_limits<float>::epsilon()) {
				*value_ptr = next_value;
				on_change(*this);
			}
		}
	}

	void update_handle() {
		if(value_ptr) {
			float t = cgv::math::clamp(*value_ptr, range[0], range[1]);
			t = (t - range[0]) / (range[1] - range[0]);
			handle.x() = rectangle.x() + static_cast<int>(t * (rectangle.w() - 10) + 0.5f);
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

	const void* get_value_ptr() const override {
		return value_ptr;
	};

	void set_value_ptr(float* ptr) {
		value_ptr = ptr;
		update_handle();
	}

	void set_range(cgv::render::vec2 range) {
		this->range = range;
		update_handle();
	}

	void update() override {
		update_handle();
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		if(!value_ptr)
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

	std::shared_ptr<text_input_control_string> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, std::string& value) {
		auto ptr = add_control<text_input_control_string>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}

	std::shared_ptr<text_input_control_float> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, float& value) {
		auto ptr = add_control<text_input_control_float>(label, position);
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



	void update_views(void* member_ptr) {
		for(auto& control : controls) {
			if(control->get_value_ptr() == member_ptr) {
				control->update();

				// post_damage()!
			}
		}
	}
};

}
}

#include <cgv/config/lib_end.h>
