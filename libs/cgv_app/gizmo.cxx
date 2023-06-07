#include "gizmo.h"

#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>
#include <cgv/math/ftransform.h>

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

		ivec2 mpos(static_cast<int>(me.get_x()), static_cast<int>(me.get_y()));
		mpos.y() = viewport_size.y() - mpos.y() - 1;
		vec2 window_coord = static_cast<vec2>(mpos) * vec2(2.0f) / static_cast<vec2>(viewport_size) - vec2(1.0f);

		mat4 MVP = ctx.get_projection_matrix() * ctx.get_modelview_matrix();

		vec4 world_coord(window_coord.x(), window_coord.y(), 1.0f, 1.0f);
		world_coord = inv(MVP) * world_coord;
		world_coord /= world_coord.w();

		ray r;
		r.org = view_ptr->get_eye();
		r.dir = normalize(vec3(world_coord) - r.org);

		switch(ma) {
		case cgv::gui::MA_MOVE:
		{
			if(active)
				break;

			bool last_hover = hover;
			InteractionFeature last_feature = feature;

			hover = intersect(r);

			if(last_hover != hover || last_feature != feature) {
				geometry_out_of_date = true;
				return true;
			}
		} break;
		case cgv::gui::MA_PRESS:
		{
			if(hover && feature != InteractionFeature::kNone) {
				if(handle_drag(r, view_dir, true)) {
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
				if(handle_drag(r, view_dir, false))
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
		scale = 0.3f * scale_coefficient * view_ptr->get_tan_of_half_of_fovy(true) * dot(position - view_ptr->get_eye(), view_ptr->get_view_dir());

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

	arrows.add(arrow_cols[0]);
	arrows.add(arrow_cols[1]);
	arrows.add(arrow_cols[2]);

	rectangles.add(v0 + 0.25f * (vy + vz), vec2(0.3f), plane_cols[0]);
	rectangles.add(v0 + 0.25f * (vx + vz), vec2(0.3f), plane_cols[1]);
	rectangles.add(v0 + 0.25f * (vx + vy), vec2(0.3f), plane_cols[2]);

	quat q = quat(vy, cgv::math::deg2rad(90.0f));
	q.normalize();
	rectangles.add(q);

	q = quat(vx, cgv::math::deg2rad(-90.0f));
	q.normalize();
	rectangles.add(q);

	q = quat();
	q.normalize();
	rectangles.add(q);

	sphere.add(v0, 0.1f, center_col);

	geometry_out_of_date = false;
}

bool gizmo::intersect_plane(const ray& r, const vec3& p, const vec3& n, float& t) const {

	float denom = dot(n, r.dir);
	if(abs(denom) < 0.0001f)
		return false;
	t = dot(p - r.org, n) / denom;
	return true;
};

bool gizmo::intersect_axis_aligned_rectangle(const ray& r, int axis, const vec3& p, float size, float& t) const {

	vec3 nml(0.0f);
	nml[axis] = 1.0f;

	float ext = size;

	if(intersect_plane(r, p, nml, t)) {
		vec3 hp = r.pos_at(t);
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

bool gizmo::intersect_sphere(const ray& r, float rad, float& t) const {

	vec3 dist = position - r.org;
	float B = dot(r.dir, dist);
	float D = B * B - dot(dist, dist) + rad * rad;

	if(D < 0.0)
		return false;

	t = B - sqrt(D);
	return true;
}

bool gizmo::intersect_cylinder(const ray& r, const vec3& pa, const vec3& pb, float rad, float& t) const {

	vec3 ba = pb - pa;
	vec3 oc = r.org - pa;

	float baba = dot(ba, ba);
	float bard = dot(ba, r.dir);
	float baoc = dot(ba, oc);

	float k2 = baba - bard * bard;
	float k1 = baba * dot(oc, r.dir) - baoc * bard;
	float k0 = baba * dot(oc, oc) - baoc * baoc - rad * rad*baba;

	float h = k1 * k1 - k2 * k0;

	if(h < 0.0)
		return false;

	h = sqrt(h);
	t = (-k1 - h) / k2;

	// body
	float y = baoc + t * bard;
	if(y > 0.0 && y < baba)
		return true;

	// caps
	t = (((y < 0.0) ? 0.0 : baba) - baoc) / bard;
	if(abs(k1 + k2 * t) < h) {
		return true;
	}

	return false;
}

bool gizmo::intersect(const ray& r) {

	float min_t = std::numeric_limits<float>::max();

	for(unsigned i = 0; i < 3; ++i) {
		vec3 pa = position;
		vec3 pb = position;
		pb[i] += 1.0f * scale * flip_factors[i];

		float t = std::numeric_limits<float>::max();
		if(intersect_cylinder(r, pa, pb, 0.05f * scale, t)) {
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

	float t = std::numeric_limits<float>::max();
	if(intersect_sphere(r, 0.1f * scale, t)) {
		if(t >= 0.0f && t < min_t) {
			min_t = t;
			feature = InteractionFeature::kCenter;
		}
	}

	return min_t > 0.0f && min_t < std::numeric_limits<float>::max();
}

bool gizmo::handle_drag(const ray& r, const vec3& view_dir, bool drag_start) {

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
	if(intersect_plane(r, plane_origin, plane_normal, t)) {
		if(drag_start) {
			last_position = position;

			if(is_axis(feature))
				offset[0] = dot(axis, r.pos_at(t) - position);
			else
				offset = r.pos_at(t) - position;
		} else {
			vec3 new_position = position;

			if(is_axis(feature)) {
				new_position[axis_idx] = dot(axis, r.pos_at(t)) - offset[0];
			} else {
				new_position = r.pos_at(t) - offset;
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
