#include "transformation_gizmo.h"

#include <cgv/math/constants.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/intersection.h>
#include <cgv/math/interpolate.h>
#include <cgv/media/named_colors.h>

using namespace cgv::render;

namespace cgv {
namespace gui {

const size_t transformation_gizmo::_k_ring_segment_count = 32;
const float transformation_gizmo::_k_full_ring_angle_threshold = 0.95f;

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

	_ring_ranges[0] = { 0, 0 };
	_ring_ranges[1] = { 0, 0 };
	_ring_ranges[2] = { 0, 0 };

	// calculate ring points for rotation handles
	cgv::math::sequence_transform(std::back_inserter(_ring_points), [](const float t) {
		return vec2(std::cos(t), std::sin(t));
	}, _k_ring_segment_count + 1, 0.0f, static_cast<float>(cgv::math::constants::pi));
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

void transformation_gizmo::init_frame(cgv::render::context& ctx) {
	set_geometry_out_of_date();
}

transformation_gizmo::Mode transformation_gizmo::get_mode() const {
	return _mode;
}

void transformation_gizmo::set_mode(Mode mode) {
	_mode = mode;
	_interaction_feature = InteractionFeature::kNone;
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

	const float base_saturation = 0.9f;
	const float hover_saturation = 0.95f;
	const float base_lightness = 0.3f;
	const float hover_lightness = 0.52f;

	auto make_color = [](const rgb& base_color, float saturation, float lightness) {
		hls color = base_color;
		color.S() = saturation;
		color.L() = lightness;
		return rgb(color);
	};

	namespace colors = cgv::media::colors;

	const std::array<rgb, 3> base_colors = {
		make_color(colors::red, base_saturation, base_lightness),
		make_color(colors::green, base_saturation, base_lightness),
		make_color(colors::blue, base_saturation, base_lightness)
	};
	const std::array<rgb, 3> hover_colors = {
		make_color(colors::red, hover_saturation, hover_lightness),
		make_color(colors::green, hover_saturation, hover_lightness),
		make_color(colors::blue, hover_saturation, hover_lightness)
	};

	auto get_color = [this, &base_colors, &hover_colors](InteractionFeature feature, AxisId axis_id) {
		return is_hovered() && _interaction_feature == feature && _interaction_axis_id == axis_id ?
			hover_colors[axis_id_to_index(axis_id)] :
			base_colors[axis_id_to_index(axis_id)];
	};

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

		const rgb x_color = get_color(InteractionFeature::kAxis, AxisId::kX);
		const rgb y_color = get_color(InteractionFeature::kAxis, AxisId::kY);
		const rgb z_color = get_color(InteractionFeature::kAxis, AxisId::kZ);

		_cones.add(v0 + _center_radius * vx, hx);
		_cones.add(v0 + _center_radius * vy, hy);
		_cones.add(v0 + _center_radius * vz, hz);
		_cones.fill_radii(_axis_radius);
		_cones.add_segment_color(x_color);
		_cones.add_segment_color(y_color);
		_cones.add_segment_color(z_color);

		float arrow_offset = has_scale ? 3.0f * _handle_size : 0.0f;

		if(has_translation) {
			// create axis handles as arrow tips
			_cones.add(hx + arrow_offset * vx, (1.0f + arrow_offset) * vx);
			_cones.add(hy + arrow_offset * vy, (1.0f + arrow_offset) * vy);
			_cones.add(hz + arrow_offset * vz, (1.0f + arrow_offset) * vz);
			_cones.add(0.5f * _handle_size, 0.0f);
			_cones.add(0.5f * _handle_size, 0.0f);
			_cones.add(0.5f * _handle_size, 0.0f);
			_cones.add_segment_color(x_color);
			_cones.add_segment_color(y_color);
			_cones.add_segment_color(z_color);
		}

		if(has_scale) {
			// create axis handles as boxes
			float box_offset = axis_length + 0.5f * _handle_size;

			_boxes.add_position(v0 + box_offset * vx);
			_boxes.add_position(v0 + box_offset * vy);
			_boxes.add_position(v0 + box_offset * vz);
			_boxes.add_color(x_color);
			_boxes.add_color(y_color);
			_boxes.add_color(z_color);
		}
	}
	
	_ring_ranges[0] = { 0, 0 };
	_ring_ranges[1] = { 0, 0 };
	_ring_ranges[2] = { 0, 0 };

	if(has_rotation) {
		// create rings
		const auto add_ring = [this, &get_color](AxisId axis_id, auto point_transform) {
			int axis_index = axis_id_to_index(axis_id);
			_ring_ranges[axis_index].lower_bound = _cones.size();

			const vec3 position_to_eye = normalize(get_view()->get_eye() - get_position());
			vec3 plane_normal = { 0.0f };
			plane_normal[axis_index] = 1.0f;

			vec3 rotated_plane_normal = get_rotation().apply(plane_normal);

			float cos_angle = cgv::math::dot(rotated_plane_normal, position_to_eye);

			if(std::abs(cos_angle) > _k_full_ring_angle_threshold) {
				for(size_t i = 0; i < 2 * _k_ring_segment_count; ++i) {
					vec2 ring_a = _ring_points[(i % _k_ring_segment_count)];
					vec2 ring_b = _ring_points[(i % _k_ring_segment_count) + 1];
					// Flip the y-axis to build the second half of the ring.
					if(i >= _k_ring_segment_count) {
						ring_a.y() = -ring_a.y();
						ring_b.y() = -ring_b.y();
					}
					vec3 a = point_transform(ring_a);
					vec3 b = point_transform(ring_b);
					_cones.add(a, b);
				}

			} else {
				vec3 projected = normalize(project_to_plane(position_to_eye, rotated_plane_normal));
				get_rotation().inverse_rotate(projected);

				vec2 plane_components = { 0.0f };
				switch(axis_id) {
				case AxisId::kX:
					plane_components = { projected.y(), projected.z() };
					break;
				case AxisId::kY:
					plane_components = { -projected.x(), projected.z() };
					break;
				case AxisId::kZ:
					plane_components = { projected.x(), projected.y() };
					break;
				}

				float angle = cgv::math::to_angle(plane_components);
				angle -= cgv::math::deg2rad(90.0f);

				quat rotation(plane_normal, angle);

				for(size_t i = 0; i < _k_ring_segment_count; ++i) {
					vec3 a = point_transform(_ring_points[i]);
					vec3 b = point_transform(_ring_points[i + 1]);
					rotation.rotate(a);
					rotation.rotate(b);
					_cones.add(a, b);
				}
			}

			_cones.fill_colors(get_color(InteractionFeature::kPlane, axis_id));
			_ring_ranges[axis_index].upper_bound = _cones.size();
		};

		add_ring(AxisId::kX, [](vec2 p) { return vec3(0.0f, p.x(), p.y()); }); // yz plane
		add_ring(AxisId::kY, [](vec2 p) { return vec3(p.x(), 0.0f, p.y()); }); // xz plane
		add_ring(AxisId::kZ, [](vec2 p) { return vec3(p.x(), p.y(), 0.0f); }); // xy plane
		_cones.fill_radii(_axis_radius);
	}

	// create plane handles
	if(_mode == Mode::kTranslation || _mode == Mode::kScale) {
		_rectangles.add_position(v0 + 0.5f * (vy + vz));
		_rectangles.add_position(v0 + 0.5f * (vx + vz));
		_rectangles.add_position(v0 + 0.5f * (vx + vy));
		_rectangles.fill_extents(vec2(_plane_size));

		_rectangles.add_rotation(quat(vy, cgv::math::deg2rad(90.0f)));
		_rectangles.add_rotation(quat(vx, cgv::math::deg2rad(-90.0f)));
		_rectangles.add_rotation(quat());

		const rgb x_color = get_color(InteractionFeature::kPlane, AxisId::kX);
		const rgb y_color = get_color(InteractionFeature::kPlane, AxisId::kY);
		const rgb z_color = get_color(InteractionFeature::kPlane, AxisId::kZ);

		_rectangles.add_color({ x_color, 0.5f });
		_rectangles.add_color({ y_color, 0.5f });
		_rectangles.add_color({ z_color, 0.5f });

		_rectangles.add_border_color(x_color);
		_rectangles.add_border_color(y_color);
		_rectangles.add_border_color(z_color);
	}

	// create center sphere
	_sphere.add(v0, _axis_radius);
	_sphere.add_color(colors::white);

	_sphere.add(v0, _center_radius);
	_sphere.colors.back().alpha() = 0.2f;
	const float center_opacity = is_hovered() && _interaction_feature == InteractionFeature::kCenter ? 0.2f : 0.0f;
	_sphere.add_color({ 0.7f, 0.7f, 0.7f, center_opacity });
}

void transformation_gizmo::draw_geometry(context& ctx) {
	// Pass the y view angle to the renderers so pixel measurements are computed correctly
	_rectangle_renderer.set_y_view_angle(static_cast<float>(get_view()->get_y_view_angle()));
	_sphere_renderer.set_y_view_angle(static_cast<float>(get_view()->get_y_view_angle()));

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
		for(int i = 0; i < 3; ++i) {
			for(size_t j = _ring_ranges[i].lower_bound; j < _ring_ranges[i].upper_bound; j += 2) {
				float t = std::numeric_limits<float>::max();
				if(cgv::math::ray_cylinder_intersection2(ray, _cones.positions[j], _cones.positions[j + 1], 3.0f * _axis_radius, t))
					update_t_if_closer(t, Mode::kRotation, InteractionFeature::kPlane, index_to_axis_id(i));
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
		for(int i = 0; i < 3; ++i) {
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
		for(int i = 0; i < 3; ++i) {
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
			// the plane is not actually needed for axis interaction, so we just choose one that will
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

	if(!_interaction_plane.is_valid())
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
				angle = static_cast<float>(2.0 * cgv::math::constants::pi) - angle;

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

} // namespace gizmo
} // namespace cgv
