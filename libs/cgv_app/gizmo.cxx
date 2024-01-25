#include "gizmo.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>
#include <cgv/math/intersection.h>

using namespace cgv::render;

namespace cgv {
namespace app {

gizmo::gizmo() {

	align_to_view = false;
	hover = false;
	active = false;
	scale = -1.0f;
	scale_coefficient = 1.0f;

	position = vec3(0.0f);
	last_position = vec3(0.0f);
	offset = vec3(0.0f);

	arrows.style.radius_relative_to_length = 0.02f;
	arrows.style.head_length_relative_to_radius = 4.0f;
	arrows.style.head_radius_scale = 2.5f;
	arrows.style.illumination_mode = IM_OFF;

	sphere.style.radius = 0.03f;
	sphere.style.illumination_mode = IM_OFF;

	rectangles.style.illumination_mode = IM_OFF;
	rectangles.style.default_border_color = rgb(0.25f);
	rectangles.style.percentual_border_width = -0.25f;

	geometry_out_of_date = true;
}

void gizmo::destruct(context& ctx) {

	renderers.arrow.clear(ctx);
	renderers.rectangle.clear(ctx);
	renderers.sphere.clear(ctx);

	arrows.destruct(ctx);
	rectangles.destruct(ctx);
	sphere.destruct(ctx);
}

bool gizmo::handle(cgv::gui::event& e, context& ctx) {

	if(!view_ptr)
		return false;

	vec3 view_dir = view_ptr->get_view_dir();

	unsigned et = e.get_kind();

	if(et == cgv::gui::EID_MOUSE) {
		cgv::gui::mouse_event& me = (cgv::gui::mouse_event&) e;
		cgv::gui::MouseAction ma = me.get_action();

		ivec2 viewport_size(ctx.get_width(), ctx.get_height());
		ivec2 mpos(
			static_cast<int>(me.get_x()),
			viewport_size.y() - static_cast<int>(me.get_y()) - 1
		);
		
		cgv::math::ray3 ray(static_cast<vec2>(mpos), static_cast<vec2>(viewport_size), view_ptr->get_eye(), ctx.get_projection_matrix() * ctx.get_modelview_matrix());

		switch(ma) {
		case cgv::gui::MA_MOVE:
		{
			if(active)
				break;

			bool last_hover = hover;
			InteractionFeature last_feature = feature;

			hover = intersect(ray);

			if(last_hover != hover || last_feature != feature) {
				geometry_out_of_date = true;
				return true;
			}
		} break;
		case cgv::gui::MA_PRESS:
		{
			if(hover && feature != InteractionFeature::kNone) {
				if(handle_drag(ray, view_dir, true)) {
					active = true;
					return true;
				}
			}
		} break;
		case cgv::gui::MA_RELEASE:
			if(active) {
				active = false;
				feature = InteractionFeature::kNone;
				geometry_out_of_date = true;
				return true;
			}
			break;
		case cgv::gui::MA_DRAG:
		{
			if(active) {
				if(handle_drag(ray, view_dir, false))
					return true;
			}
		} break;
		default: break;
		}
	}

	return false;
}

bool gizmo::init(context& ctx) {

	bool success = true;

	success &= renderers.arrow.init(ctx);
	success &= renderers.rectangle.init(ctx);
	success &= renderers.sphere.init(ctx);

	success &= arrows.init(ctx);
	success &= rectangles.init(ctx);
	success &= sphere.init(ctx);

	return true;
}

void gizmo::draw(context& ctx) {

	if(!view_ptr)
		return;

	if(geometry_out_of_date)
		create_geometry();

	if(!active || scale < 0.0f)
		scale = 0.3f * scale_coefficient * static_cast<float>(view_ptr->get_tan_of_half_of_fovy(true)) * dot(position - view_ptr->get_eye(), view_ptr->get_view_dir());

	if(!active && align_to_view)
		flip_factors = get_flip_factors();
	else
		flip_factors = vec3(1.0f);

	ctx.push_modelview_matrix();
	ctx.mul_modelview_matrix(cgv::math::translate4(position));
	ctx.mul_modelview_matrix(cgv::math::scale4(vec3(scale) * flip_factors));

	arrows.render(ctx, renderers.arrow);
	rectangles.render(ctx, renderers.rectangle);
	sphere.render(ctx, renderers.sphere);

	ctx.pop_modelview_matrix();
}

void gizmo::set_view_ptr(view* view_ptr) {

	this->view_ptr = view_ptr;
}

void gizmo::create_geometry() {

	const rgb grey = rgb(0.5f, 0.5f, 0.5f);
	const rgb dark_grey = rgb(0.25f, 0.25f, 0.25f);
	const rgb red = rgb(1.0f, 0.0f, 0.0f);
	const rgb green = rgb(0.0f, 1.0f, 0.0f);
	const rgb blue = rgb(0.0f, 0.0f, 1.0f);
	const rgb yellow = rgb(1.0f, 1.0f, 0.0f);

	arrows.clear();
	rectangles.clear();
	sphere.clear();

	vec3 v0(0.0f);
	vec3 vx(1.0f, 0.0f, 0.0f);
	vec3 vy(0.0f, 1.0f, 0.0f);
	vec3 vz(0.0f, 0.0f, 1.0f);

	vec3 color_multiplier(0.75f);
	vec3 color_add(0.0f);

	rgb arrow_cols[3] = {
		0.85f * red,
		0.85f * green,
		0.85f * blue
	};

	rgb plane_cols[3] = {
		grey,
		grey,
		grey
	};

	rgb center_col = dark_grey;

	int axis_idx = get_axis_idx(feature);
	if(hover) {
		if(axis_idx > -1) {
			if(is_axis(feature)) {
				color_multiplier[axis_idx] = 0.5f;
				color_add[axis_idx] = 1.0f;
				arrow_cols[axis_idx] = yellow;
			} else if(is_plane(feature)) {
				plane_cols[axis_idx] = yellow;
			}
		} else {
			if(is_center(feature)) {
				center_col = yellow;
			}
		}
	}

	arrows.add(v0, vx);
	arrows.add(v0, vy);
	arrows.add(v0, vz);

	arrows.add_color(arrow_cols[0]);
	arrows.add_color(arrow_cols[1]);
	arrows.add_color(arrow_cols[2]);

	rectangles.add(v0 + 0.25f * (vy + vz), plane_cols[0], vec2(0.3f));
	rectangles.add(v0 + 0.25f * (vx + vz), plane_cols[1], vec2(0.3f));
	rectangles.add(v0 + 0.25f * (vx + vy), plane_cols[2], vec2(0.3f));

	quat rotation = quat(vy, cgv::math::deg2rad(90.0f));
	rectangles.add_rotation(rotation);

	rotation = quat(vx, cgv::math::deg2rad(-90.0f));
	rectangles.add_rotation(rotation);

	rotation = quat();
	rectangles.add_rotation(rotation);

	sphere.add(v0, center_col, 0.1f);

	geometry_out_of_date = false;
}

bool gizmo::intersect_axis_aligned_rectangle(const cgv::math::ray3& r, int axis, const vec3& p, float size, float& t) const {

	vec3 nml(0.0f);
	nml[axis] = 1.0f;

	float ext = size;

	if(cgv::math::ray_plane_intersection(r, p, nml, t)) {
		vec3 hp = r.position(t);
		hp -= p;

		vec2 uv;
		switch(axis) {
		case 0:
			uv[0] = hp[1];
			uv[1] = hp[2];
			break;
		case 1:
			uv[0] = hp[0];
			uv[1] = hp[2];
			break;
		case 2:
			uv[0] = hp[0];
			uv[1] = hp[1];
			break;
		default:
			return false;
			break;
		}

		uv += 0.5f*ext;

		if(uv[0] >= 0.0f && uv[0] <= ext && uv[1] >= 0.0f && uv[1] <= ext) {
			return true;
		}
	}

	return false;
}

bool gizmo::intersect(const cgv::math::ray3& r) {

	float min_t = std::numeric_limits<float>::max();

	for(unsigned i = 0; i < 3; ++i) {
		vec3 pa = position;
		vec3 pb = position;
		pb[i] += 1.0f * scale * flip_factors[i];

		float t = std::numeric_limits<float>::max();
		if(cgv::math::ray_cylinder_intersection2(r, pa, pb, 0.05f * scale, t)) {
			if(t >= 0.0f && t < min_t) {
				min_t = t;
				feature = static_cast<InteractionFeature>(i + 1);
			}
		}
	}

	for(unsigned i = 0; i < 3; ++i) {
		vec3 p = position + 0.25f * scale * flip_factors;
		p[i] = position[i];

		float t = std::numeric_limits<float>::max();
		if(intersect_axis_aligned_rectangle(r, i, p, 0.3f * scale, t)) {
			if(t >= 0.0f && t < min_t) {
				min_t = t;
				feature = static_cast<InteractionFeature>(i + 4);
			}
		}
	}

	vec2 ts(std::numeric_limits<float>::max());	
	if(cgv::math::ray_sphere_intersection(r, position, 0.1f * scale, ts)) {
		if(ts.x() >= 0.0f && ts.x() < min_t) {
			min_t = ts.x();
			feature = InteractionFeature::kCenter;
		}
	}

	return min_t > 0.0f && min_t < std::numeric_limits<float>::max();
}

bool gizmo::handle_drag(const cgv::math::ray3& r, const vec3& view_dir, bool drag_start) {

	int axis_idx = get_axis_idx(feature);
	vec3 axis = get_axis(feature);

	vec3 plane_origin = drag_start ? position : last_position;
	vec3 plane_normal = vec3(0.0f);

	if(is_axis(feature)) {
		vec3 angs;
		for(unsigned i = 0; i < 3; ++i) {
			float ang = -1.0f;
			if(i != axis_idx) {
				vec3 v(0.0f);
				v[i] = 1.0f;
				ang = abs(dot(v, view_dir));
			}
			angs[i] = ang;
		}

		plane_normal[cgv::math::max_index(angs)] = 1.0f;
	} else if(is_plane(feature)) {
		plane_normal = axis;
	} else if(is_center(feature)) {
		plane_normal = view_dir;
	} else {
		return false;
	}

	float t = -1.0f;
	if(cgv::math::ray_plane_intersection(r, plane_origin, plane_normal, t)) {
		if(drag_start) {
			last_position = position;

			if(is_axis(feature))
				offset[0] = dot(axis, r.position(t) - position);
			else
				offset = r.position(t) - position;
		} else {
			vec3 new_position = position;

			if(is_axis(feature)) {
				new_position[axis_idx] = dot(axis, r.position(t)) - offset[0];
			} else {
				new_position = r.position(t) - offset;
			}

			set_position(new_position, true);
		}

		return true;
	}

	return false;
}

vec3 gizmo::get_flip_factors() {

	vec3 factors(1.0f);

	if(view_ptr) {
		vec3 delta = view_ptr->get_eye() - position;

		if(delta[0] < 0.0f)
			factors[0] = -1.0f;
		if(delta[1] < 0.0f)
			factors[1] = -1.0f;
		if(delta[2] < 0.0f)
			factors[2] = -1.0f;
	}

	return factors;
}

}
}
