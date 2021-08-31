#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/render/render_types.h>

#include "rect.h"

#include "../lib_begin.h"

namespace cgv {
namespace glutil {

template<class T>
class draggables_collection : public cgv::render::render_types {
protected:
	typedef typename std::remove_pointer<T>::type T_;
	typedef typename T_* T_ptr;

	static_assert(std::is_base_of<draggable, T_>::value, "T must inherit from draggable");

	T_ptr get_ptr(T_& obj) { return &obj; }
	T_ptr get_ptr(T_ptr obj) { return obj; }

	bool has_constraint = false;
	rect constraint_area;

	std::vector<T> draggables;

	T_ptr dragged;
	T_ptr selected;

	ivec2 offset;

	std::function<void(void)> drag_start_callback;
	std::function<void(void)> drag_callback;
	std::function<void(void)> drag_end_callback;

	T_ptr get_hit_draggable(const ivec2& pos) {
		T_ptr hit = nullptr;
		for(unsigned i = 0; i < draggables.size(); ++i) {
			T_ptr d = get_ptr(draggables[i]);

			if(d && d->is_inside(pos))
				hit = d;
		}
		return hit;
	}

public:
	draggables_collection() {
		dragged = nullptr;
		selected = nullptr;
	}

	void add(T obj) {
		draggables.push_back(obj);
	}

	size_t size() { return draggables.size(); }

	std::vector<T>& ref_draggables() { return draggables; }

	T operator[](int i) {
		return draggables[i];
	}

	T get_dragged() {
		return dragged;
	}

	T get_selected() {
		return selected;
	}

	void set_constraint(const rect& area) {
		constraint_area = area;
		has_constraint = true;
	}

	void remove_constraint() {
		constraint_area = cgv::glutil::overlay::rect();
		has_constraint = false;
	}

	void set_drag_start_callback(std::function<void(void)> func) {
		drag_start_callback = func;
	}

	void set_drag_callback(std::function<void(void)> func) {
		drag_callback = func;
	}

	void set_drag_end_callback(std::function<void(void)> func) {
		drag_end_callback = func;
	}

	bool handle(cgv::gui::event& e, const ivec2& viewport_size) {
		unsigned et = e.get_kind();
		unsigned char modifiers = e.get_modifiers();

		if(et == cgv::gui::EID_MOUSE) {
			cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
			cgv::gui::MouseAction ma = me.get_action();

			ivec2 mpos(me.get_x(), me.get_y());
			mpos.y() = viewport_size.y() - mpos.y();

			if(me.get_button() == cgv::gui::MB_LEFT_BUTTON) {
				if(ma == cgv::gui::MA_RELEASE) {
					if(dragged) {
						selected = dragged;
						dragged = nullptr;
					} else {
						selected = get_hit_draggable(mpos);
					}
					if(drag_end_callback) drag_end_callback();
				}
			}

			if(me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
				if(dragged) {
					dragged->pos = mpos + offset;
					if(has_constraint)
						dragged->apply_constraint(constraint_area);
					if(drag_callback) drag_callback();
				} else {
					if(ma == cgv::gui::MA_PRESS) {
						dragged = get_hit_draggable(mpos);
						selected = dragged;
						if(dragged) {
							offset = dragged->pos - mpos;
							if(drag_start_callback) drag_start_callback();
						}
					}
				}
				return true;
			}

			return false;
		} else {
			return false;
		}
	}
};

}
}

#include <cgv/config/lib_end.h>
