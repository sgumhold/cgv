#include "transformation_gizmo.h"

#include <cgv/math/intersection.h>

using namespace cgv::render;

namespace cgv {
namespace app {

transformation_gizmo::transformation_gizmo() {
	_boxes.style.illumination_mode = IM_OFF;
	_boxes.style.map_color_to_material = CM_COLOR_AND_OPACITY;
	_boxes.style.default_extent = _handle_size;

	_cones.style.illumination_mode = IM_OFF;
	_cones.style.map_color_to_material = CM_COLOR_AND_OPACITY;
	_cones.style.radius = 0.02f;

	_sphere.style.illumination_mode = IM_OFF;
	_sphere.style.map_color_to_material = CM_COLOR_AND_OPACITY;
	_sphere.style.halo_color = { 1.0f };

	_rectangles.style.illumination_mode = IM_OFF;
	_rectangles.style.map_color_to_material = CM_COLOR_AND_OPACITY;

	// calculate ring points for rotation handles
	for(size_t i = 0; i < _ring_segment_count; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(_ring_segment_count - 1);
		t *= 2.0f * M_PI;
		_ring_points.push_back({ std::cos(t), std::sin(t) });
	}
	_ring_points.back() = _ring_points.front();
}

bool transformation_gizmo::init(context& ctx) {
	bool success = true;

	success &= _box_renderer.init(ctx);
	success &= _cone_renderer.init(ctx);
	success &= _rectangle_renderer.init(ctx);
	success &= _sphere_renderer.init(ctx);

	success &= _boxes.init(ctx);
	success &= _cones.init(ctx);
	success &= _rectangles.init(ctx);
	success &= _sphere.init(ctx);

	return success;
}

void transformation_gizmo::clear(context& ctx) {
	_box_renderer.clear(ctx);
	_cone_renderer.clear(ctx);
	_rectangle_renderer.clear(ctx);
	_sphere_renderer.clear(ctx);

	_boxes.destruct(ctx);
	_cones.destruct(ctx);
	_rectangles.destruct(ctx);
	_sphere.destruct(ctx);
}

transformation_gizmo::Mode transformation_gizmo::get_mode() const {
	return _mode;
}

void transformation_gizmo::set_mode(Mode mode) {
	_mode = mode;
	set_geometry_out_of_date();
	post_redraw();
}

vec3 transformation_gizmo::get_scale() const {
	return _scale;
}

void transformation_gizmo::set_scale(const vec3& scale) {
	_scale = scale;
	post_redraw();
}

void transformation_gizmo::create_geometry() {
	using hls = cgv::media::color<float, cgv::media::ColorModel::HLS>;

	const vec3 v0(0.0f);
	const vec3 vx(1.0f, 0.0f, 0.0f);
	const vec3 vy(0.0f, 1.0f, 0.0f);
	const vec3 vz(0.0f, 0.0f, 1.0f);

	hls red = rgb(1.0f, 0.0f, 0.0f);
	hls green = rgb(0.0f, 1.0f, 0.0f);
	hls blue = rgb(0.0f, 0.0f, 1.0f);

	const float saturation = 0.9f;
	const float lightness = 0.3f;

	red.S() = saturation;
	green.S() = saturation;
	blue.S() = saturation;
	red.L() = lightness;
	green.L() = lightness;
	blue.L() = lightness;

	const rgb x_color = red;
	const rgb y_color = green;
	const rgb z_color = blue;

	_boxes.clear();
	_cones.clear();
	_rectangles.clear();
	_sphere.clear();

	bool use_axes =
		_mode == Mode::kTranslation ||
		_mode == Mode::kScale ||
		_mode == Mode::kModel;

	bool has_translation =
		_mode == Mode::kTranslation ||
		_mode == Mode::kModel;

	bool has_rotation =
		_mode == Mode::kRotation ||
		_mode == Mode::kModel;

	bool has_scale =
		_mode == Mode::kScale ||
		_mode == Mode::kModel;

	if(use_axes) {
		float axis_length = 1.0f;

		if(has_translation && has_scale)
			axis_length -= 2.0f * _handle_size;
		else if(has_translation)
			axis_length -= 2.0f * _handle_size;
		else if(has_scale)
			axis_length -= _handle_size;

		vec3 hx = axis_length * vx;
		vec3 hy = axis_length * vy;
		vec3 hz = axis_length * vz;

		_cones.add(v0 + _center_radius * vx, hx);
		_cones.add(v0 + _center_radius * vy, hy);
		_cones.add(v0 + _center_radius * vz, hz);
		_cones.fill_radii(_axis_radius);
		_cones.add_segment_color({ x_color, 1.0f });
		_cones.add_segment_color({ y_color, 1.0f });
		_cones.add_segment_color({ z_color, 1.0f });

		float arrow_offset = has_scale ? 3.0f * _handle_size : 0.0f;

		if(has_translation) {
			// create axis handles as arrow tips
			_cones.add(hx + arrow_offset * vx, (1.0f + arrow_offset) * vx);
			_cones.add(hy + arrow_offset * vy, (1.0f + arrow_offset) * vy);
			_cones.add(hz + arrow_offset * vz, (1.0f + arrow_offset) * vz);
			_cones.add(0.5f * _handle_size, 0.0f);
			_cones.add(0.5f * _handle_size, 0.0f);
			_cones.add(0.5f * _handle_size, 0.0f);
			_cones.add_segment_color({ x_color, 1.0f });
			_cones.add_segment_color({ y_color, 1.0f });
			_cones.add_segment_color({ z_color, 1.0f });
		}

		if(has_scale) {
			// create axis handles as boxes
			float box_offset = axis_length + 0.5f * _handle_size;

			_boxes.add_position(v0 + box_offset * vx);
			_boxes.add_position(v0 + box_offset * vy);
			_boxes.add_position(v0 + box_offset * vz);
			_boxes.add_color({ x_color, 1.0f });
			_boxes.add_color({ y_color, 1.0f });
			_boxes.add_color({ z_color, 1.0f });
		}
	}
	
	if(has_rotation) {
		// create rings
		const auto add_ring = [this](auto transform, const rgba& color) {
			for(size_t i = 0; i < _ring_points.size(); ++i)
				_cones.add(transform(_ring_points[i]), transform(_ring_points[(i + 1) % _ring_points.size()]));
			_cones.fill_colors(color);
		};

		add_ring([](vec2 p) { return vec3(0.0f, p.x(), p.y()); }, { x_color, 1.0f }); // yz plane
		add_ring([](vec2 p) { return vec3(p.x(), 0.0f, p.y()); }, { y_color, 1.0f }); // xz plane
		add_ring([](vec2 p) { return vec3(p.x(), p.y(), 0.0f); }, { z_color, 1.0f }); // xy plane
		_cones.fill_radii(_axis_radius);
	}

	// create plane handles
	if(_mode == Mode::kTranslation || _mode == Mode::kScale) {
		_rectangles.add_position(v0 + 0.5f * (vy + vz));
		_rectangles.add_position(v0 + 0.5f * (vx + vz));
		_rectangles.add_position(v0 + 0.5f * (vx + vy));
		_rectangles.fill_extents(vec2(_plane_size));

		_rectangles.add_color({ x_color, 0.5f });
		_rectangles.add_color({ y_color, 0.5f });
		_rectangles.add_color({ z_color, 0.5f });

		_rectangles.add_border_color({ x_color, 1.0f });
		_rectangles.add_border_color({ y_color, 1.0f });
		_rectangles.add_border_color({ z_color, 1.0f });

		_rectangles.add_rotation(quat(vy, cgv::math::deg2rad(90.0f)));
		_rectangles.add_rotation(quat(vx, cgv::math::deg2rad(-90.0f)));
		_rectangles.add_rotation(quat());
	}

	// create center sphere
	_sphere.add(v0, _axis_radius);
	_sphere.colors.push_back({ 1.0f });

	_sphere.add(v0, _center_radius);
	_sphere.colors.push_back({ 0.7f, 0.7f, 0.7f, 0.0f });
	
	if(!is_hovered())
		return;

	// set colors based on hover state
	const auto saturate_color = [](rgba& color) {
		cgv::media::color<float, cgv::media::ColorModel::HLS> hls = color;
		hls.S() = 0.95f;
		hls.L() = 0.52f;
		color = { rgb(hls), color.alpha() };
	};

	int axis_idx = axis_id_to_index(_interaction_axis_id);
	switch(_interaction_feature) {
	case InteractionFeature::kAxis:
		if(use_axes) {
			size_t base_idx = 2.0f * axis_idx;
			if(has_translation && has_scale && _interaction_mode == Mode::kScale || has_translation != has_scale) {
				saturate_color(_cones.colors[base_idx]);
				saturate_color(_cones.colors[base_idx + 1]);
			}

			if(_interaction_mode == Mode::kTranslation) {
				saturate_color(_cones.colors[base_idx + 6]);
				saturate_color(_cones.colors[base_idx + 7]);
			}

			if(_interaction_mode == Mode::kScale)
				saturate_color(_boxes.colors[axis_idx]);
		}
		break;
	case InteractionFeature::kPlane:
		if(has_rotation) {
			size_t base_idx = static_cast<size_t>(axis_idx) * 2 * _ring_segment_count;
			if(_mode == Mode::kModel)
				base_idx += 12;

			for(size_t i = 0; i < 2 * _ring_segment_count; ++i)
				saturate_color(_cones.colors[base_idx + i]);
		} else {
			saturate_color(_rectangles.colors[axis_idx]);
			saturate_color(_rectangles.border_colors[axis_idx]);
		}
		break;
	case InteractionFeature::kCenter:
		_sphere.colors.back().alpha() = 0.2f;
		break;
	default:
		break;
	}
}

void transformation_gizmo::draw_geometry(context& ctx) {
	// Pass the y view angle to the renderers so pixel measurements are computed correctly
	_rectangle_renderer.set_y_view_angle(get_view()->get_y_view_angle());
	_sphere_renderer.set_y_view_angle(get_view()->get_y_view_angle());

	// Since we use scaling in the modelview matrix we also need to adjust the pixel measures
	// in the render styles based on the scale of the gizmo.
	const float size = get_size();

	_sphere.style.halo_width_in_pixel = -3.0f / size;
	_sphere.style.blend_width_in_pixel = 1.0f / size;

	_rectangles.style.border_width_in_pixel = -3.0f / size;
	_rectangles.style.pixel_blend = 2.0f / size;

	_sphere.render(ctx, _sphere_renderer);
	_rectangles.render(ctx, _rectangle_renderer);
	_cones.render(ctx, _cone_renderer);
	_boxes.render(ctx, _box_renderer);
}

bool transformation_gizmo::intersect_bounding_box(const cgv::math::ray3& ray) {
	vec3 min = { 0.0f };
	vec3 max = { 1.0f };

	if(_mode == Mode::kRotation || _mode == Mode::kModel)
		min = -1.0f;
	else
		min -= _center_radius;

	if(_mode == Mode::kModel)
		max += 3.0f * _handle_size;

	min -= 0.1f;
	max += 0.1f;

	vec2 t = std::numeric_limits<float>::max();
	return cgv::math::ray_box_intersection(ray, min, max, t) != 0;
}

bool transformation_gizmo::intersect(const cgv::math::ray3& ray) {
	float min_t = std::numeric_limits<float>::max();

	const auto update_t_if_closer = [this, &min_t](float t, Mode transformation, InteractionFeature feature, AxisId axis_id) {
		if(t >= 0.0f && t < min_t) {
			min_t = t;
			_interaction_mode = transformation;
			_interaction_feature = feature;
			_interaction_axis_id = axis_id;
		}
	};

	if(_mode == Mode::kRotation || _mode == Mode::kModel) {
		// test rings for planes
		size_t start_offset = _mode == Mode::kModel ? 12 : 0;
		for(size_t i = start_offset; i < _cones.size(); i += 2) {
			vec3 pa = _cones.positions[i];
			vec3 pb = _cones.positions[i + 1];

			float t = std::numeric_limits<float>::max();
			if(cgv::math::ray_cylinder_intersection2(ray, pa, pb, 3.0f * _axis_radius, t)) {
				int axis_idx = static_cast<int>((i - start_offset) / (2 * _ring_segment_count));
				update_t_if_closer(t, Mode::kRotation, InteractionFeature::kPlane, index_to_axis_id(axis_idx));
			}
		}
	}

	
	std::array<std::pair<vec3, vec3>, 6> cylinders;
	cylinders.fill({ 0.0f, 0.0f });
	size_t cylinder_count = 0;
	float axis_length = 1.0f;

	if(_mode == Mode::kModel) {
		axis_length -= _handle_size;
	}

	if(_mode == Mode::kTranslation || _mode == Mode::kScale || _mode == Mode::kModel) {
		cylinder_count += 3;
		for(unsigned i = 0; i < 3; ++i) {
			cylinders[i].first[i] = _center_radius;
			cylinders[i].second[i] = axis_length;
		}
	}

	if(_mode == Mode::kModel) {
		cylinder_count += 3;
		for(size_t i = 0; i < 3; ++i) {
			cylinders[i + 3].first[i] = axis_length + 2.0f * _handle_size;
			cylinders[i + 3].second[i] = axis_length + 4.0f * _handle_size;
		}
	}
	
	for(size_t i = 0; i < cylinder_count; ++i) {
		float t = std::numeric_limits<float>::max();
		if(cgv::math::ray_cylinder_intersection2(ray, cylinders[i].first, cylinders[i].second, _handle_size, t)) {
			Mode transformation = _mode;
			if(cylinder_count > 3)
				transformation = i < 3 ? Mode::kScale : Mode::kTranslation;

			update_t_if_closer(t, transformation, InteractionFeature::kAxis, index_to_axis_id(static_cast<int>(i % 3)));
		}
	}

	// test rectangles for planes
	if(_mode == Mode::kTranslation || _mode == Mode::kScale) {
		float t = std::numeric_limits<float>::max();
		for(size_t i = 0; i < 3; ++i) {
			vec3 position = { 0.5f };
			position[i] = 0.0f;
			if(cgv::math::ray_axis_aligned_rectangle_intersection(ray, position, { _plane_size }, static_cast<int>(i), t))
				update_t_if_closer(t, _mode, InteractionFeature::kPlane, index_to_axis_id(static_cast<int>(i)));
		}
	}

	// test sphere for center
	vec2 ts(std::numeric_limits<float>::max());
	if(cgv::math::ray_sphere_intersection(ray, { 0.0f }, _center_radius, ts)) {
		Mode transformation = _mode == Mode::kModel ? Mode::kTranslation : _mode;
		update_t_if_closer(ts.x(), transformation, InteractionFeature::kCenter, AxisId::kX);
	}

	return min_t > 0.0f && min_t < std::numeric_limits<float>::max();
}

bool transformation_gizmo::start_drag(const cgv::math::ray3& ray) {
	int axis_idx = axis_id_to_index(_interaction_axis_id);
	vec3 axis = get_axis(axis_idx);

	const vec3 view_dir = get_view()->get_view_dir();

	_interaction_plane.origin = get_position();

	const auto get_rotated_axis = [this](const vec3& axis) {
		vec3 rotated = axis;
		get_rotation().rotate(rotated);
		return rotated;
	};

	if(_interaction_mode == Mode::kRotation) {
		switch(_interaction_feature) {
		case InteractionFeature::kPlane:
		{
			_interaction_plane.normal = get_rotated_axis(axis);
			// if the ray direction is close to parallel to the plane, choose the screen aligned plane instead
			float threshold_angle = std::cos(cgv::math::deg2rad(75.0f));

			float incident_angle = dot(_interaction_plane.normal, ray.direction);
			if(std::abs(incident_angle) < threshold_angle)
				_interaction_plane.normal = incident_angle < 0.0f ? -view_dir : view_dir;
			break;
		}
		case InteractionFeature::kCenter:
			_interaction_plane.normal = view_dir;
			break;
		default:
			return false;
		}
	} else {
		switch(_interaction_feature) {
		case InteractionFeature::kAxis:
		{
			// the plane is not actually needed for axis interaction, so we just chose one that will
			// always produce an intersection with the mouse ray
			_interaction_plane.normal = view_dir;
			break;
		}
		case InteractionFeature::kPlane:
			_interaction_plane.normal = get_rotated_axis(axis);
			break;
		case InteractionFeature::kCenter:
			_interaction_plane.normal = view_dir;
			break;
		default:
			return false;
		}
	}

	float t = -1.0f;
	if(!cgv::math::ray_plane_intersection(ray, _interaction_plane.origin, _interaction_plane.normal, t))
		return false;

	_drag_start_t = t;
	_drag_start_position = get_position();
	_drag_start_scale = _scale;
	_drag_start_rotation = get_rotation();

	if(on_change)
		on_change(GizmoAction::kDragStart, _interaction_mode);

	return true;
}

bool transformation_gizmo::drag(const cgv::math::ray3& ray) {
	int axis_idx = axis_id_to_index(_interaction_axis_id);
	vec3 axis = get_axis(axis_idx);

	const vec3 position = get_position();

	if(!_interaction_plane.valid())
		return false;

	float t = -1.0f;
	if(!cgv::math::ray_plane_intersection(ray, _interaction_plane.origin, _interaction_plane.normal, t) || t < 0.0f) {
		// restore drag start transforms if no valid intersection with the interaction plane is found
		set_position(_drag_start_position);
		set_scale(_drag_start_scale);
		set_rotation(_drag_start_rotation);
	} else {
		vec3 start_intersection_position = _drag_start_ray.position(_drag_start_t);
		vec3 intersection_position = ray.position(t);

		if(get_orientation() == GizmoOrientation::kLocal)
			_drag_start_rotation.rotate(axis);

		switch(_interaction_mode) {
		case Mode::kTranslation:
		{
			// TODO: FIXME: This is not optimal and only works well when the mouse position is pointing close to the manipulated axis.
			cgv::math::ray3 axis_ray = { _drag_start_position, axis };

			float start_offset = ray_ray_closest_approach(_drag_start_ray, axis_ray).second;
			float offset = ray_ray_closest_approach(ray, axis_ray).second;

			vec3 new_position = _drag_start_position;
			if(_interaction_feature == InteractionFeature::kAxis)
				new_position += (offset - start_offset) * axis;
			else
				new_position += intersection_position - start_intersection_position;// _local_offset;

			set_position(new_position);
			break;
		}
		case Mode::kScale:
		{
			// TODO: FIXME: Scaling does not work corectly for rotated coordinate frames (local gizmo orientation).
			vec3 new_local_offset = start_intersection_position - _drag_start_position;
			if(_interaction_feature == InteractionFeature::kAxis)
				new_local_offset[axis_idx] = (intersection_position - position)[axis_idx];
			else
				new_local_offset = intersection_position - position;

			vec3 scale_mult = new_local_offset / (start_intersection_position - _drag_start_position);

			vec3 new_scale = _drag_start_scale;
			switch(_interaction_feature) {
			case InteractionFeature::kAxis:
				new_scale[axis_idx] *= scale_mult[axis_idx];
				break;
			case InteractionFeature::kPlane:
				for(int i = 0; i < 3; ++i) {
					if(i != axis_idx)
						new_scale[i] *= scale_mult[i];
				}
				break;
			case InteractionFeature::kCenter:
				new_scale *= length(scale_mult);
				break;
			default:
				break;
			}

			set_scale(new_scale);
			break;
		}
		case Mode::kRotation:
		{
			vec3 start_dir = start_intersection_position - _drag_start_position;
			vec3 end_dir = intersection_position - position;

			// check length and skip if too short
			if(start_dir.normalize() < 0.01f)
				return true;
			if(end_dir.normalize() < 0.01f)
				return true;

			vec3 plane_tangent = normalize(cross(_interaction_plane.normal, start_dir));

			float cos_theta = dot(start_dir, end_dir);
			cos_theta = cgv::math::clamp(cos_theta, -1.0f, 1.0f);
			float angle = std::acos(cos_theta);

			// check for side and bring angle in range [0,2pi];
			if(dot(plane_tangent, end_dir) < 0.0f)
				angle = 2.0f * M_PI - angle;

			vec3 rotaton_axis = _interaction_feature == InteractionFeature::kCenter ? _interaction_plane.normal : axis;

			quat new_rotation = quat(rotaton_axis, angle) * _drag_start_rotation;

			set_rotation(new_rotation);
			break;
		}
		default:
			return false;
		}
	}
	
	if(on_change)
		on_change(GizmoAction::kDrag, _interaction_mode);

	return true;
}

void transformation_gizmo::end_drag(const cgv::math::ray3& ray) {
	if(on_change)
		on_change(GizmoAction::kDragEnd, _interaction_mode);
}

std::pair<float, float> transformation_gizmo::ray_ray_closest_approach(const cgv::math::ray3& r0, const cgv::math::ray3& r1) const {
	vec3 ba = r1.direction;
	vec3 oa = r0.origin - r1.origin;

	float a = dot(ba, ba);
	float b = dot(r0.direction, ba);
	float c = dot(oa, ba);
	float e = dot(oa, r0.direction);

	vec2 st = vec2(c - b * e, b * c - a * e) / (a - b * b);

	return { st.y(), st.x() };
}

}
}
