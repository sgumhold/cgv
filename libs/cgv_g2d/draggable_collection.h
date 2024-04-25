#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/mouse_event.h>

#include "canvas.h"
#include "draggable.h"
#include "trect.h"
#include "utils2d.h"

#include "lib_begin.h"

namespace cgv {
namespace g2d {

template<class T>
class draggable_collection {
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

	bool press_inside;
	ptr_type dragged;
	ptr_type selected;

	ivec2 offset;

	std::function<void(void)> drag_start_callback;
	std::function<void(void)> drag_callback;
	std::function<void(void)> drag_end_callback;
	std::function<void(void)> selection_change_callback;

	ptr_type get_hit_draggable(const ivec2& pos) {
		ptr_type hit = nullptr;
		for(unsigned i = 0; i < draggables.size(); ++i) {
			ptr_type d = get_ptr(draggables[i]);

			if(d && d->contains(pos))
				hit = d;
		}
		return hit;
	}

	bool handle_void(cgv::gui::event& e, const ivec2& viewport_size, const irect& container, Origin origin) {
		unsigned et = e.get_kind();

		if(et == cgv::gui::EID_MOUSE) {
			cgv::gui::mouse_event& me = (cgv::gui::mouse_event&)e;

			vec2 mouse_position = static_cast<vec2>(cgv::g2d::get_local_mouse_pos(ivec2(me.get_x(), me.get_y()), viewport_size, container, origin));
			return handle_mouse_event(me, mouse_position);
		}

		return false;
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

	bool empty() const {
		return draggables.empty();
	}

	size_t size() const { return draggables.size(); }

	std::vector<T>& ref_draggables() { return draggables; }

	accessor_type operator[](size_t i) {
		return draggables[i];
	}

	typename std::vector<T>::iterator begin() { return draggables.begin(); }

	typename std::vector<T>::iterator end() { return draggables.end(); }

	typename std::vector<T>::const_iterator cbegin() { return draggables.cbegin(); }

	typename std::vector<T>::const_iterator cend() { return draggables.cend(); }

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

	void clear_selected() {
		selected = nullptr;
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
	
	void set_selection_change_callback(std::function<void(void)> func) {
		selection_change_callback = func;
	}

	void set_transformation(const mat3& matrix) {
		inv_transformation = cgv::math::inv(matrix);
	}

	bool handle_mouse_event(cgv::gui::mouse_event& me, vec2 mouse_position) {
		mouse_position = vec2(inv_transformation * vec3(mouse_position, 1.0f));

		if(me.get_button() == cgv::gui::MB_LEFT_BUTTON) {
			if(me.get_action() == cgv::gui::MA_PRESS) {
				press_inside = has_constraint && !use_individual_constraints ? constraint_area.contains(mouse_position) : true;
				dragged = get_hit_draggable(mouse_position);

				press_inside = press_inside || dragged;

				if(press_inside) {
					selected = dragged;
					if(dragged) {
						offset = dragged->position - mouse_position;
						if(drag_start_callback) drag_start_callback();
						return true;
					}
				}
			} else if(me.get_action() == cgv::gui::MA_RELEASE) {
				if(dragged) {
					dragged = nullptr;
					if(drag_end_callback) drag_end_callback();
				} else if(press_inside) {
					selected = get_hit_draggable(mouse_position);
					if(selection_change_callback) selection_change_callback();
				}
			}
		}

		if(me.get_button_state() & cgv::gui::MB_LEFT_BUTTON) {
			if(dragged) {
				dragged->position = mouse_position + offset;

				if(use_individual_constraints && dragged->get_constraint())
					dragged->apply_constraint();
				else if(has_constraint)
					dragged->apply_constraint(constraint_area);

				if(drag_callback) drag_callback();
				return true;
			}
		}
		
		return false;
	}

	bool handle(cgv::gui::event& e, const ivec2& viewport_size, Origin origin = Origin::kBottomLeft) {
		return handle_void(e, viewport_size, irect(), origin);
	}

	bool handle(cgv::gui::event& e, const ivec2& viewport_size, const irect& container, Origin origin = Origin::kBottomLeft) {
		return handle_void(e, viewport_size, container, origin);
	}

	bool handle(cgv::gui::event& e, const ivec2& viewport_size, const canvas& cnvs) {
		return handle_void(e, viewport_size, irect(), cnvs.get_origin_setting());
	}

	bool handle(cgv::gui::event& e, const ivec2& viewport_size, const canvas& cnvs, const irect& container) {
		return handle_void(e, viewport_size, container, cnvs.get_origin_setting());
	}
};

}
}

#include <cgv/config/lib_end.h>
