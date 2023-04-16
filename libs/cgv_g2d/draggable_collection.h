#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/render/render_types.h>

#include "rect.h"
#include "draggable.h"

#include "lib_begin.h"

namespace cgv {
namespace g2d {

template<class T>
class draggable_collection : public cgv::render::render_types {
protected:
	typedef typename std::remove_pointer<T>::type raw_type;
	typedef          raw_type* ptr_type;
	typedef typename std::conditional<std::is_pointer<T>::value, ptr_type, raw_type&>::type accessor_type;

	static_assert(std::is_base_of<draggable, raw_type>::value, "T must inherit from draggable");

	ptr_type get_ptr(raw_type& obj) { return &obj; }
	ptr_type get_ptr(ptr_type obj) { return obj; }

	bool has_constraint = false;
	bool use_individual_constraints = false;
	irect constraint_area;
	mat3 inv_transformation;

	std::vector<T> draggables;

	ptr_type dragged;
	ptr_type selected;

	ivec2 offset;

	std::function<void(void)> drag_start_callback;
	std::function<void(void)> drag_callback;
	std::function<void(void)> drag_end_callback;

	ptr_type get_hit_draggable(const ivec2& pos) {
		ptr_type hit = nullptr;
		for(unsigned i = 0; i < draggables.size(); ++i) {
			ptr_type d = get_ptr(draggables[i]);

			if(d && d->is_inside(pos))
				hit = d;
		}
		return hit;
	}

public:
	draggable_collection() {
		clear();
	}

	void clear() {
		dragged = nullptr;
		selected = nullptr;
		draggables.clear();
		inv_transformation.identity();
	}

	void add(T obj) {
		draggables.push_back(obj);
	}

	size_t size() const { return draggables.size(); }

	std::vector<T>& ref_draggables() { return draggables; }

	accessor_type operator[](size_t i) {
		return draggables[i];
	}

	ptr_type get_dragged() {
		return dragged;
	}

	void set_dragged(int i) {
		if(i >= 0 && i < draggables.size())
			dragged = get_ptr(draggables[i]);
	}

	ptr_type get_selected() {
		return selected;
	}

	void set_selected(int i) {
		if(i >= 0 && i < draggables.size())
			selected = get_ptr(draggables[i]);
	}

	const irect& get_constraint() const { return constraint_area; }
	
	void set_constraint(const irect& area) {
		constraint_area = area;
		has_constraint = true;
	}

	void remove_constraint() {
		constraint_area = irect();
		has_constraint = false;
	}

	void set_use_individual_constraints(bool flag) {
		use_individual_constraints = true;
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

	void set_transformation(const mat3& matrix) {
		inv_transformation = cgv::math::inv(matrix);
	}

	bool handle(cgv::gui::event& e, const ivec2& viewport_size, const irect& container = irect()) {
		unsigned et = e.get_kind();
		unsigned char modifiers = e.get_modifiers();

		if(et == cgv::gui::EID_MOUSE) {
			cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
			cgv::gui::MouseAction ma = me.get_action();

			ivec2 mpos(me.get_x(), me.get_y());
			mpos.y() = viewport_size.y() - mpos.y() - 1;
			mpos -= container.pos();

			vec3 tmp = inv_transformation * vec3(mpos, 1.0f);
			mpos.x() = static_cast<int>(tmp.x());
			mpos.y() = static_cast<int>(tmp.y());

			if(me.get_button() == cgv::gui::MB_LEFT_BUTTON) {
				if(ma == cgv::gui::MA_PRESS) {
					dragged = get_hit_draggable(mpos);
					selected = dragged;
					if(dragged) {
						offset = dragged->pos - mpos;
						if(drag_start_callback) drag_start_callback();
						return true;
					}
				} else if(ma == cgv::gui::MA_RELEASE) {
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
					if(use_individual_constraints) {
						if(dragged->get_constraint())
							dragged->apply_constraint();
						else
							if(has_constraint)
								dragged->apply_constraint(constraint_area);
					} else {
						if(has_constraint)
							dragged->apply_constraint(constraint_area);
					}
					if(drag_callback) drag_callback();
					return true;
				}
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
