#include "gizmo.h"

#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/intersection.h>

using namespace cgv::render;

namespace cgv {
namespace gui {

void gizmo::finish_frame(context& ctx) {
	if(!_view)
		_view = find_view_as_node();

	if(!_view)
		return;

	if(_geometry_out_of_date) {
		create_geometry();
		_geometry_out_of_date = false;
	}
	
	if(use_screen_size) {
		const vec3 eye_to_position = _position - static_cast<vec3>(_view->get_eye());
		const vec3 view_direction = static_cast<vec3>(_view->get_view_dir());
		const float perpendicular_distance = length(project_to_vector(eye_to_position, view_direction));
		const float visible_height = 2.0f * static_cast<float>(_view->get_tan_of_half_of_fovy(true)) * perpendicular_distance;

		const ivec4& viewport = ctx.get_window_transformation_array().back().viewport;
		const float adjusted_size = screen_size * visible_height / static_cast<float>(viewport.w());

		if(lock_screen_size_during_interaction) {
			if(!_captured_mouse || _size < 0.0f)
				_size = adjusted_size;
		} else {
			_size = adjusted_size;
		}
	} else {
		_size = world_size;
	}

	ctx.push_depth_test_state();
	ctx.disable_depth_test();

	ctx.push_blend_state();
	ctx.enable_blending();
	ctx.set_blend_func_back_to_front();

	ctx.push_modelview_matrix();
	ctx.mul_modelview_matrix(cgv::math::translate4(_position));
	ctx.mul_modelview_matrix(cgv::math::scale4(_size));

	if(_orientation == GizmoOrientation::kLocal)
		ctx.mul_modelview_matrix(_rotation.get_homogeneous_matrix());

	draw_geometry(ctx);

	ctx.pop_modelview_matrix();

	ctx.pop_blend_state();

	ctx.pop_depth_test_state();
}

bool gizmo::handle(cgv::gui::event& e) {
	if(!cgv::render::drawable::get_active() || e.get_kind() != cgv::gui::EID_MOUSE)
		return false;

	cgv::gui::mouse_event& me = dynamic_cast<cgv::gui::mouse_event&>(e);

	context* ctx = get_context();
	if(!_view || !ctx)
		return false;

	const ivec4& viewport = ctx->get_window_transformation_array().front().viewport;
	vec2 viewport_size = vec2(viewport.z(), viewport.w());

	// get the mouse position in GL space
	ivec2 mouse_pos(
		static_cast<int>(me.get_x()),
		viewport_size.y() - static_cast<int>(me.get_y()) - 1
	);

	cgv::math::ray3 ray(vec2(mouse_pos), viewport_size, _view->get_eye(), ctx->get_projection_matrix() * ctx->get_modelview_matrix());

	bool evaluate_hover_state = false;

	switch(me.get_action()) {
	case cgv::gui::MA_MOVE:
		evaluate_hover_state = !_captured_mouse;
		break;
	case cgv::gui::MA_PRESS:
		if(me.get_button() == cgv::gui::MouseButton::MB_LEFT_BUTTON && _hovered && _interaction_feature != InteractionFeature::kNone) {
			if(start_drag(ray)) {
				_drag_start_ray = ray;
				_captured_mouse = true;
				return true;
			}
		}
		break;
	case cgv::gui::MA_RELEASE:
		if(_captured_mouse && me.get_button() == cgv::gui::MouseButton::MB_LEFT_BUTTON) {
			end_drag(ray);
			_captured_mouse = false;
			evaluate_hover_state = true;
		}
		break;
	case cgv::gui::MA_DRAG:
		if(_captured_mouse && me.get_button_state() == cgv::gui::MouseButton::MB_LEFT_BUTTON && drag(ray))
			return true;
		break;
	default:
		break;
	}

	if(evaluate_hover_state) {
		// transform ray to gizmo object space
		cgv::math::ray3 ray_object = ray;
		ray_object.origin -= _position;
		ray_object.origin /= _size;

		if(_orientation == GizmoOrientation::kLocal) {
			_rotation.inverse_rotate(ray_object.origin);
			_rotation.inverse_rotate(ray_object.direction);
		}

		bool was_hovered = _hovered;
		InteractionFeature last_feature = _interaction_feature;
		AxisId last_axis_id = _interaction_axis_id;

		_hovered = intersect_bounding_box(ray_object) && intersect(ray_object);

		if(was_hovered != _hovered || last_feature != _interaction_feature || last_axis_id != _interaction_axis_id) {
			set_geometry_out_of_date();
			post_redraw();
		}
	}

	return false;
}

GizmoOrientation gizmo::get_orientation() const {
	return _orientation;
}

void gizmo::set_orientation(GizmoOrientation orientation) {
	_orientation = orientation;
	post_redraw();
}

vec3 gizmo::get_position() const {
	return _position;
}

void gizmo::set_position(const vec3& position) {
	_position = position;
	post_redraw();
}

quat gizmo::get_rotation() const {
	return _rotation;
}

void gizmo::set_rotation(const quat& rotation) {
	_rotation = rotation;
	post_redraw();
}

float gizmo::get_size() const {
	return _size;
}

const cgv::render::view* gizmo::get_view() const {
	return _view;
}

void gizmo::set_geometry_out_of_date() {
	_geometry_out_of_date = true;
}

bool gizmo::captured_mouse() const {
	return _captured_mouse;
}

bool gizmo::is_hovered() const {
	return _hovered;
}

vec3 gizmo::get_axis(int index) const {
	assert(index >= 0 && index < 3);
	vec3 axis(0.0f);
	axis[index] = 1.0f;
	return axis;
}

vec3 gizmo::get_axis_mask(InteractionFeature feature, AxisId axis_id) const {
	switch(feature) {
	case InteractionFeature::kAxis:
		return get_axis(axis_id_to_index(axis_id));
	case InteractionFeature::kPlane:
		return vec3(1.0f) - get_axis(axis_id_to_index(axis_id));
	case InteractionFeature::kCenter:
		return vec3(1.0f);
	default:
		return vec3(0.0f);
	}
}

int gizmo::axis_id_to_index(AxisId axis) const {
	return static_cast<int>(axis);
}

gizmo::AxisId gizmo::index_to_axis_id(int idx) const {
	assert(idx >= 0 && idx < 3);
	return static_cast<AxisId>(idx);
}

} // namespace gizmo
} // namespace cgv
