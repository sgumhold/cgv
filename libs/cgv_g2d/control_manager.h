#pragma once

#include <memory>

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



#include <cgv/gui/control.h>
#include <cgv/type/variant.h>
#include <cgv/type/info/type_id.h>


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
private:
	typedef std::function<void(control_base*, void*)> callback_t;
	callback_t _callback;
	void* _user_data = nullptr;

protected:
	std::string label;
	irect rectangle;

public:
	control_base(const std::string& label, irect rectangle) : label(label), rectangle(rectangle) {}

	virtual ~control_base() {}

	void set_label(const std::string& label) {
		this->label = label;
	}

	virtual void update() = 0;

	void callback(callback_t c, void* p) {
		_callback = c;
		_user_data = p;
	}

	void do_callback() {
		if(_callback)
			_callback(this, _user_data);
	}

	virtual bool handle_key_event(cgv::gui::key_event& e) { return false; }
	virtual bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) { return false; }

	virtual void draw(cgv::render::context& ctx, canvas& cnvs, const control_styles& styles) {}
};


class CGV_API value_control : public control_base {
private:
	double value = 0.0;
	cgv::render::dvec2 range = cgv::render::dvec2(0.0, 1.0);
	double step = 1.0;

public:
	using control_base::control_base;

	double get_value() const {
		return value;
	}

	bool set_value(double v) {
		if(v == value)
			return false;

		value = v;
		update();
		return true;
	}
	
	cgv::render::dvec2 get_range() const {
		return range;
	}

	void set_range(cgv::render::dvec2 range) {
		this->range = range;

		// TODO: udpate value to be in range
		update();
	}

	double get_step() const {
		return step;
	}

	void set_step(double step) {
		this->step = step;
	}

	void handle_value_change(double v) {
		// round to nearest multiple of step
		if(step >= 1.0) {
			v = std::round(v / step) * step;
		} else if(step > 0.0) {
			// Try to detect fractions like .1 which are actually stored as
			// .9999999 and thus would round to unexpected values. This is done
			// by seeing if 1/N is close to an integer:
			double is = std::round(1.0 / step);
			if(std::abs(is * step - 1.0) < 0.001)
				v = std::round(v * is) / is;
			else
				v = std::round(v / step) * step;
		} else {
			// check for them incrementing toward zero and don't produce tiny
			// numbers:
			//if(previous_value_ && fabs(v / previous_value_) < 1e-5) v = 0;
		}

		// If original value was in-range, clamp the new value:
		double min = range[0];
		double max = range[1];
		
		if(min > max)
			std::swap(min, max);

		//if(v < A && previous_value_ >= A)
		//	v = A;
		//else if(v > B && previous_value_ <= B)
		//	v = B;
		// 
		// store the value, redraw the widget, and do callback:
		if(set_value(v))
			do_callback();
	}
};






















/*
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
class CGV_API input_control : public control_base {
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

	virtual bool input_valid(char c) {
		return std::isprint(c);
	}

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
	*

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
					if(input_valid(c))
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

	cgv::signal::signal<input_control&> on_change;
};

class CGV_API string_input_control : public input_control<std::string> {
protected:
	void update_value() override {
		if(value_ptr)
			*value_ptr = text;
	}

public:
	using input_control<std::string>::input_control;

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

class CGV_API float_input_control : public input_control<float> {
protected:
	void update_value() override {
		if(value_ptr)
			// TODO: exception handling
			*value_ptr = std::stof(text);
	}

	bool input_valid(char c) override {
		return std::isdigit(c) || c == '.';
	}

public:
	using input_control<float>::input_control;

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

class CGV_API int_input_control : public input_control<int> {
protected:
	void update_value() override {
		if(value_ptr)
			// TODO: exception handling
			*value_ptr = static_cast<int>(std::stol(text));
	}

	bool input_valid(char c) override {
		return std::isdigit(c);
	}

public:
	using input_control<int>::input_control;

	void set_value_ptr(int* ptr) override {
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
*/



/*
//template<typename T>
class CGV_API slider_control : public value_control {
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
	slider_control(const std::string& label, irect rectangle) : value_control(label, rectangle) {
		handle.position = static_cast<cgv::render::vec2>(rectangle.position);
		handle.size = cgv::render::vec2(12.0f, static_cast<float>(rectangle.h()));
		handle_draggable.add(&handle);
		handle_draggable.set_constraint(rectangle);
		handle_draggable.set_drag_callback(std::bind(&slider_control::update_value, this));
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

	cgv::signal::signal<slider_control&> on_change;
};
*/









class CGV_API input_control : public control_base {
public:
	enum class input_type {
		kString,
		kFloat,
		kInteger
	} type = input_type::kString;
private:
	bool focused = false;
	bool do_focus = false;
	float focus_position;
	size_t cursor_position = 0;

	std::string buffer;

	bool is_allowed(char c) {
		switch(type) {
		case input_type::kFloat: return std::isdigit(c) || c == '.' || c == 'e';
		case input_type::kInteger: return std::isdigit(c);
		case input_type::kString:
		default: return std::isprint(c);
		}
	}

	void erase_at_cursor() {
		buffer.erase(cursor_position, 1);
		
		do_callback();
	}

	void insert_at_cursor(char c) {
		if(is_allowed(c)) {
			buffer.insert(cursor_position, 1, c);
			++cursor_position;

			do_callback();
		}
	}

public:
	using control_base::control_base;

	std::string get_value() const {
		return buffer;
	}

	bool set_value(const std::string& v) {
		if(v == buffer)
			return false;
		
		buffer = v;

		cursor_position = std::min(cursor_position, buffer.length());
		update();
		return true;
	}

	void update() override {}

	bool handle_key_event(cgv::gui::key_event& e) override {
		cgv::gui::KeyAction action = e.get_action();

		if(focused && (action == cgv::gui::KA_PRESS || action == cgv::gui::KA_REPEAT)) {
			switch(e.get_key()) {
			case cgv::gui::KEY_Home:
				cursor_position = 0;
				return true;
			case cgv::gui::KEY_End:
				cursor_position = buffer.length();
				return true;
			case cgv::gui::KEY_Left:
				if(cursor_position > 0)
					--cursor_position;
				return true;
			case cgv::gui::KEY_Right:
				if(cursor_position < buffer.length())
					++cursor_position;
				return true;
			case cgv::gui::KEY_Back_Space:
				if(cursor_position > 0 && cursor_position <= buffer.length()) {
					--cursor_position;
					erase_at_cursor();
				}
				return true;
			case cgv::gui::KEY_Delete:
				if(cursor_position < buffer.length())
					erase_at_cursor();
				return true;
			case cgv::gui::KEY_Space:
				insert_at_cursor(' ');
				return true;
			case '\\':
				// capture escape sequences, e.g, when pressing '^'
				return true;
			default:
				if(e.get_key() < 256) {
					//std::cout << "key = " << ke.get_key() << ", char = '" << ke.get_char() << "', char_num = " << static_cast<int>(ke.get_char()) << std::endl;
					insert_at_cursor(e.get_char());
					return true;
				}
				break;
			}
		}

		return false;
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		cgv::gui::MouseAction action = e.get_action();

		if(e.get_button() == cgv::gui::MB_LEFT_BUTTON) {
			if(action == cgv::gui::MouseAction::MA_PRESS) {
				if(rectangle.is_inside(mouse_position)) {
					focused = true;
					do_focus = true;
					focus_position = mouse_position.x();
					cursor_position = buffer.length();
					return true;
				} else if(focused) {
					// TODO: do we even need to deactivate the input? FLTK for example only removes the caret when the mouse leaves the gui
					focused = false;
					cursor_position = 0;
					// TOOD: should issue redraw but not cancel event
					//return true;
				}
			}
		}

		return false;
	}

	void draw(cgv::render::context& ctx, canvas& cnvs, const control_styles& styles) override {
		auto& font = ref_msdf_font_regular(ctx);

		if(do_focus) {
			do_focus = false;
			cursor_position = 0;

			float position = rectangle.x() + 5.0f;
			float closest_distance = std::abs(position - focus_position);
			float last_distance = closest_distance;

			for(size_t i = 0; i < buffer.length(); ++i) {
				const auto& glyph = font.get_glyph_info(static_cast<unsigned char>(buffer[i]));

				position += glyph.advance * styles.text.font_size;
				float distance = std::abs(position - focus_position);

				if(distance < closest_distance) {
					closest_distance = distance;
					cursor_position = i + 1;
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
		ref_msdf_gl_canvas_font_renderer(ctx).render(ctx, cnvs, font, buffer, styles.text, position + cgv::render::ivec2(5, 0), cgv::render::TA_LEFT);

		if(focused) {
			cnvs.enable_shader(ctx, "rectangle");
			cnvs.set_style(ctx, styles.colored_box);

			cgv::render::vec2 text_size = styles.text.font_size * cgv::render::vec2(font.compute_length(buffer, cursor_position), 1.0f);

			cgv::render::vec2 position = rectangle.position;
			position.x() += text_size.x() + 5.0f;
			position.x() = std::floor(position.x()) + 0.5f;
			position.y() += 2.0f;

			cnvs.draw_shape(ctx, position, cgv::render::vec2(1.0f, 16.0f), styles.text.fill_color);
			cnvs.disable_current_shader(ctx);
		}
	}
};


class CGV_API value_input_control : public value_control {
public:
	input_control input;

	value_input_control(const std::string& label, irect rectangle);

	bool set_value(double v) {
		char buf[32];
		if(std::floor(v) == v) {
			snprintf(buf, 32, "%d", static_cast<int>(v));
		} else {
			// this is a very brute force way to allow 6 digits to the right instead
			// of the %g default of 4:
			int n = (int)ceil(log10(fabs(v)));
			if(n > 0 || n < -6) {
				snprintf(buf, 32, "%g", v);
			} else {
				snprintf(buf, 32, "%.7f", v);
				// strip trailing 0's and the period:
				char* s = buf; while(*s) s++; s--;
				while(s > buf && *s == '0') s--;
				if(*s == '.') s--;
				s[1] = 0;
			}
		}

		std::string str(buf);

		input.set_value(str);


		return value_control::set_value(v);
	}

	
	void update() override {}

	bool handle_key_event(cgv::gui::key_event& e) override {
		return input.handle_key_event(e);
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
		return input.handle_mouse_event(e, mouse_position);
	}

	void draw(cgv::render::context& ctx, canvas& cnvs, const control_styles& styles) override {
		input.draw(ctx, cnvs, styles);
	}
};



















// TODO: move to math lib and add cgv::math::clamp with arguments (T, fvec<T, 2>)
template<typename T>
T map(T value, cgv::math::fvec<T, 2>& in_range, cgv::math::fvec<T, 2>& out_range) {
	return out_range[0] + (out_range[1] - out_range[0]) * ((value - in_range[0]) / (in_range[1] - in_range[0]));
}

class CGV_API slider_control : public value_control {
private:
	cgv::g2d::draggable handle;
	cgv::g2d::draggable_collection<cgv::g2d::draggable*> handle_draggable;

	void update_value() {
		//auto& range = get_range();
		//double t = (handle.x() - rectangle.x()) / (rectangle.w() - handle.w());
		//double next_value = cgv::math::lerp(range[0], range[1], t);
		//next_value = cgv::math::clamp(next_value, range[0], range[1]);

		cgv::render::dvec2 in_range(rectangle.x(), rectangle.x1() - handle.w());
		double next_value = map(static_cast<double>(handle.x()), in_range, get_range());
		next_value = cgv::math::clamp(next_value, get_range()[0], get_range()[1]);

		handle_value_change(next_value);
	}

	void update_handle() {
		//auto& range = get_range();
		//double t = cgv::math::clamp(get_value(), range[0], range[1]);
		//t = (t - range[0]) / (range[1] - range[0]);
		//handle.x() = rectangle.x() + static_cast<int>(t * (rectangle.w() - handle.w()) + 0.5f);

		cgv::render::dvec2 out_range(rectangle.x(), rectangle.x1() - handle.w());
		double position = static_cast<float>(map(get_value(), get_range(), out_range));
		handle.x() = static_cast<float>(cgv::math::clamp(position, out_range[0], out_range[1]));// +0.5f;
	}

public:
	slider_control(const std::string& label, irect rectangle) : value_control(label, rectangle) {
		handle.position = static_cast<cgv::render::vec2>(rectangle.position);
		handle.size = cgv::render::vec2(12.0f, static_cast<float>(rectangle.h()));
		handle_draggable.add(&handle);
		handle_draggable.set_constraint(rectangle);
		handle_draggable.set_drag_callback(std::bind(&slider_control::update_value, this));
	}

	void update() override {
		update_handle();
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
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
};















/*
class CGV_API value_input_control : public value_control {
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

	cgv::g2d::draggable handle;
	cgv::g2d::draggable_collection<cgv::g2d::draggable*> handle_draggable;

	void update_value() {
		//auto& range = get_range();
		//double t = (handle.x() - rectangle.x()) / (rectangle.w() - handle.w());
		//double next_value = cgv::math::lerp(range[0], range[1], t);
		//next_value = cgv::math::clamp(next_value, range[0], range[1]);

		cgv::render::dvec2 in_range(rectangle.x(), rectangle.x1() - handle.w());
		double next_value = map(static_cast<double>(handle.x()), in_range, get_range());
		next_value = cgv::math::clamp(next_value, get_range()[0], get_range()[1]);

		handle_value_change(next_value);
	}

	void update_handle() {
		//auto& range = get_range();
		//double t = cgv::math::clamp(get_value(), range[0], range[1]);
		//t = (t - range[0]) / (range[1] - range[0]);
		//handle.x() = rectangle.x() + static_cast<int>(t * (rectangle.w() - handle.w()) + 0.5f);

		cgv::render::dvec2 out_range(rectangle.x(), rectangle.x1() - handle.w());
		double position = static_cast<float>(map(get_value(), get_range(), out_range));
		handle.x() = static_cast<float>(cgv::math::clamp(position, out_range[0], out_range[1]));// +0.5f;
	}

protected:
	std::string buffer;

	virtual void update_value() = 0;

	virtual bool input_valid(char c) {

	}

public:
	using value_control::value_control;

	void update() override {
		update_handle();
	}

	bool handle_key_event(cgv::gui::key_event& e) override {
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
					if(input_valid(c))//return std::isprint(c);
						insert_character(c);
					return true;
				}
				break;
			}
		}

		return false;
	}

	bool handle_mouse_event(cgv::gui::mouse_event& e, cgv::render::ivec2 mouse_position) override {
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

		if(do_focus) {
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
	}
};
*/














































/** the fltk base class keeps strings for label and tooltip and
	implements methods corresponding to the property interface of
	 cgv::base::base.*/
struct CGV_API gl_control_base {
	gl_control_base() {}

	/*
	/// store the cursor
	fltk::Cursor* cursor;
	/// store the tooltip as string
	std::string tooltip;
	// store the alignment as string
	std::string alignment;
	// default width and height of the element
	int default_width, default_height;
	// minimal width and height of the element
	int minimum_width, minimum_height;
	/// returns declarations for the reflected properties of a fltk Widget
	std::string get_property_declarations();
	/// set the property of a fltk widget
	bool set_void(fltk::Widget* w, cgv::base::named* nam, const std::string& property, const std::string& type, const void* value);
	/// get the property of a fltk widget 
	bool get_void(const fltk::Widget* w, cgv::base::named* nam, const std::string& property, const std::string& type, void* value);
	/// handle method that ensures that the cursor is shown correctly
	int handle(fltk::Widget* w, int event);
	*/
};





/// this interface is used to update the value of fltk_value_control s.
struct CGV_API abst_gl_value_callback {
	/// interface for value updates independent of the value and fltk Valuator type 
	virtual void update_value_if_valid(double v) = 0;
};




// TODO: move to source file and remove static specifier (maybe use a method inside gl_value_control with std::bind and get rid of external cb and abst_gl_value_callback?)
static void valuator_cb(control_base* control, void* valuator_ptr) {
	abst_gl_value_callback* value_callback = dynamic_cast<abst_gl_value_callback*>(static_cast<cgv::base::base*>(valuator_ptr));
	if(value_callback)
		value_callback->update_value_if_valid(static_cast<value_control*>(control)->get_value());
}




/** The control<T> is implemented with different fltk Valuator
	widgets. The fltk_value_control is parameterized over
	 the fltk Valuator control widget type FC and the type T
	 of the conrolled value. */
template <typename T, typename GC>
struct CGV_API gl_value_control : public cgv::gui::control<T>, public abst_gl_value_callback, public gl_control_base {
	/// pointer to the fltk Widget that controls the value
	// TODO: find good name
	std::shared_ptr<GC> gl_control;
	/// construct from label, value reference and dimensions
	gl_value_control(const std::string& label, T& value, irect rectangle) : cgv::gui::control<T>(label, &value) {
		gl_control = std::make_shared<GC>(this->get_name(), rectangle);
		
		// TODO: implement to set value and range
		//configure(value, fC);

		//gl_control->callback(valuator_cb, static_cast<cgv::base::base*>(this));
		gl_control->callback(valuator_cb, this);
		update();
	}
	/// destruct fltk value control
	~gl_value_control() {}
	/// returns "fltk_value_control"
	std::string get_type_name() const {
		return "gl_value_control";
	}
	/// updates the fltk control widget in case the controlled value has been changed externally
	void update() {
		T tmp = this->get_value();
		gl_control->set_value(cgv::type::variant<double>::get(cgv::type::info::type_name<T>::get_name(), &tmp));
	}
	/// adds to the implementation of fltk_base based on the control type
	//std::string get_property_declarations();
	/// abstract interface for the setter
	//bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	//bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer
	// TODO: do we ned this?
	void* get_user_data() const {
		return static_cast<control_base*>(gl_control.get());
	}
	/// interface for value updates independent of the value type T
	void update_value_if_valid(double v) {
		if(this->check_and_set_value((T)v) && gl_control->get_value() != this->get_value())
			update();
	}


	std::shared_ptr<GC> get_gl_control() {
		return gl_control;
	}
};




/** implements a control for string values with the fltk::Input class */
struct CGV_API gl_string_control : public cgv::gui::control<std::string>, public gl_control_base {
	bool in_callback = false;
	/// a fltk::Input is used to implement the string control
	std::shared_ptr<input_control> gl_control;
	/// construct from label, value reference and dimensions
	gl_string_control(const std::string& label, std::string& value, irect rectangle);
	/// destruct fltk input
	~gl_string_control() {}
	/// give access to the protected value ptr to allow changing the value
	void public_set_value(const std::string& value) {
		set_value(value);
	}
	/// returns "fltk_string_control"
	std::string get_type_name() const {
		return "gl_string_control";
	}
	/// updates the fltk::Input widget in case the controled value has been changed externally
	void update() {
		if(!in_callback)
			gl_control->set_value(this->get_value());
	}
	/// only uses the implementation of fltk_base
	//std::string get_property_declarations();
	/// abstract interface for the setter
	//bool set_void(const std::string& property, const std::string& value_type, const void* value_ptr);
	/// abstract interface for the getter
	//bool get_void(const std::string& property, const std::string& value_type, void* value_ptr);
	/// return a fltk::Widget pointer
	void* get_user_data() const {
		return nullptr;
	}

	std::shared_ptr<input_control> get_gl_control() {
		return gl_control;
	}
};














































class CGV_API control_manager : cgv::gui::theme_observer {
protected:
	cgv::render::shader_library shaders;

	control_styles styles;
	cgv::render::ivec2 default_control_size = cgv::render::ivec2(200, 20);

	std::vector<std::shared_ptr<control_base>> controls;
	std::vector<cgv::gui::control_ptr> controls2;

	std::function<void(void*)> default_on_change_callback;

	void init_styles();

	void handle_theme_change(const cgv::gui::theme_info& theme) override;

public:
	control_manager();

	bool init(cgv::render::context& ctx);

	void destruct(cgv::render::context& ctx);

	void clear();

	bool handle(cgv::gui::event& e, const cgv::render::ivec2& viewport_size, const irect& container = irect());

	void draw(cgv::render::context& ctx, cgv::g2d::canvas& cnvs);



	
	/*
	std::shared_ptr<button_control> add_button(const std::string& label, cgv::render::ivec2 position) {
		return add_control<button_control>(label, position);
	}

	std::shared_ptr<string_input_control> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, std::string& value) {
		auto ptr = add_control<string_input_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}

	std::shared_ptr<float_input_control> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, float& value) {
		auto ptr = add_control<float_input_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}

	std::shared_ptr<int_input_control> add_text_input(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, int& value) {
		auto ptr = add_control<int_input_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}
	*/

	/*
	std::shared_ptr<slider_control> add_slider_control(cgv::base::base* base_ptr, const std::string& label, cgv::render::ivec2 position, float& value) {
		auto ptr = add_control<slider_control>(label, position);
		ptr->set_value_ptr(&value);
		connect_copy(ptr->on_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, &value));
		return ptr;
	}
	*/


	//template <class B>
	std::shared_ptr<slider_control> add_slider_control(cgv::base::base* base_ptr, const std::string& label, float* value_ptr, cgv::render::ivec2 position) {
		auto gl_control_ptr = new gl_value_control<cgv::type::flt32_type, /* B */slider_control>(
			label, *static_cast<cgv::type::flt32_type*>(value_ptr), irect(position, default_control_size)
		);

		data::ref_ptr<cgv::gui::control<float>> control_ptr = gl_control_ptr;

		if(control_ptr)
			connect_copy(control_ptr->value_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, value_ptr));

		controls2.push_back(cgv::gui::control_ptr(gl_control_ptr));

		auto control_view_ptr = gl_control_ptr->get_gl_control();

		controls.push_back(control_view_ptr);
		return control_view_ptr;
	}

	std::shared_ptr<input_control> add_string_control(cgv::base::base* base_ptr, const std::string& label, std::string* value_ptr, cgv::render::ivec2 position) {
		auto gl_control_ptr = new gl_string_control(
			label, *value_ptr, irect(position, default_control_size)
		);

		data::ref_ptr<cgv::gui::control<std::string>> control_ptr = gl_control_ptr;

		if(control_ptr)
			connect_copy(control_ptr->value_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, value_ptr));

		controls2.push_back(cgv::gui::control_ptr(gl_control_ptr));

		auto control_view_ptr = gl_control_ptr->get_gl_control();

		controls.push_back(control_view_ptr);
		return control_view_ptr;
	}

	std::shared_ptr<value_input_control> add_value_control(cgv::base::base* base_ptr, const std::string& label, float* value_ptr, cgv::render::ivec2 position) {
		auto gl_control_ptr = new gl_value_control<cgv::type::flt32_type, value_input_control>(
			label, *static_cast<cgv::type::flt32_type*>(value_ptr), irect(position, default_control_size)
		);

		data::ref_ptr<cgv::gui::control<float>> control_ptr = gl_control_ptr;

		if(control_ptr)
			connect_copy(control_ptr->value_change, cgv::signal::rebind(base_ptr, &cgv::base::base::on_set, value_ptr));

		controls2.push_back(cgv::gui::control_ptr(gl_control_ptr));

		auto control_view_ptr = gl_control_ptr->get_gl_control();

		controls.push_back(control_view_ptr);
		return control_view_ptr;
	}
};

}
}

#include <cgv/config/lib_end.h>
