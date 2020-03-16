#include "sphere_container.h"

namespace cgv {
	namespace nui {

sphere_container::sphere_container(bool use_radii, bool _use_colors, SphereRenderType _render_type)
	: primitive_container(PT_SPHERE, _use_colors, false, use_radii?SM_UNIFORM:SM_NONE), render_type(_render_type)
{
	srs.radius = 1.0f;
	srs.radius_scale = 1.0f;
}

uint32_t sphere_container::add_sphere(const vec3& center)
{
	center_positions.push_back(center);
	if (use_colors)
		colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
	if (scaling_mode == SM_UNIFORM)
		uniform_scales.push_back(1.0f);
	return uint32_t(center_positions.size() - 1);
}

uint32_t sphere_container::add_sphere(const vec3& center, float radius)
{
	center_positions.push_back(center);
	if (use_colors)
		colors.push_back(rgba(0.5f, 0.5f, 0.5f, 1));
	if (scaling_mode == SM_UNIFORM)
		uniform_scales.push_back(radius);
	return uint32_t(center_positions.size() - 1);
}
uint32_t sphere_container::add_sphere(const vec3& center, const rgba& color)
{
	center_positions.push_back(center);
	if (use_colors)
		colors.push_back(color);
	if (scaling_mode == SM_UNIFORM)
		uniform_scales.push_back(1.0f);
	return uint32_t(center_positions.size() - 1);
}
uint32_t sphere_container::add_sphere(const vec3& center, const rgba& color, float radius)
{
	center_positions.push_back(center);
	if (use_colors)
		colors.push_back(color);
	if (scaling_mode == SM_UNIFORM)
		uniform_scales.push_back(radius);
	return uint32_t(center_positions.size() - 1);
}

void sphere_container::add_strip(uint32_t start_index, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		indices.push_back(start_index + i);
	indices.push_back(uint32_t(-1));
}

sphere_container::box3 sphere_container::compute_bounding_box() const
{
	vec3 one(1.0f);
	box3 B;
	for (const auto& c : center_positions) {
		float r = get_radius(uint32_t(&c - &center_positions.front()));
		B.add_point(c - r * one);
		B.add_point(c + r * one);
	}
	return B;
}

void sphere_container::compute_closest_point(contact_info& info, const vec3& pos)
{
	for (const auto& c : center_positions) {
		uint32_t i = uint32_t(&c - &center_positions.front());
		float r = get_radius(i);
		vec3 n = normalize(pos - c);
		vec3 p = c + r*n;
		float distance = (p - pos).length();
		consider_closest_point(i, info, distance, p, n);
	}
}

void sphere_container::compute_closest_oriented_point(contact_info& info, const vec3& pos, const vec3& normal, float orientation_weight)
{
	primitive_container::compute_closest_oriented_point(info, pos, normal, orientation_weight);
	// TODO: implement this
}

int sphere_container::compute_intersection(const vec3& ce, float ra, const vec3& ro, const vec3& rd, contact_info::contact& C, contact_info::contact* C2_ptr)
{
	vec3 oc = ro - ce;
	float b = dot(oc, rd);
	float c = dot(oc, oc) - ra * ra;
	float h = b * b - c;
	if (h < 0.0)
		return 0;
	if (h < 0.0000001f) {
		if (b >= 0.0f)
			return 0;
		C.distance = -b;
		C.position = ro - b * rd;
		C.normal = normalize(C.position - ce);
		return 1;
	}
	h = sqrt(h);
	if (-b - h < 0) {
		if (-b + h < 0)
			return 0;
		C.distance = -b+h;
		C.position = ro + C.distance * rd;
		C.normal = normalize(C.position - ce);
		return 1;
	}
	C.distance = -b - h;
	C.position = ro + C.distance * rd;
	C.normal = normalize(C.position - ce);
	if (C2_ptr) {
		C2_ptr->distance = -b + h;
		C2_ptr->position = ro + C2_ptr->distance * rd;
		C2_ptr->normal = normalize(C2_ptr->position - ce);
	}
	return 2;
}

void sphere_container::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
{
	for (const auto& c : center_positions) {
		uint32_t i = uint32_t(&c - &center_positions.front());
		float r = get_radius(i);
		contact_info::contact ci;
		if (compute_intersection(c, r, start, direction, ci) > 0) {
			ci.primitive_index = i;
			ci.container = this;
			if (info.contacts.empty())
				info.contacts.push_back(ci);
			else if (ci.distance < info.contacts.front().distance)
				info.contacts.front() = ci;
		}
	}
}

void sphere_container::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
{
	for (const auto& c : center_positions) {
		uint32_t i = uint32_t(&c - &center_positions.front());
		float r = get_radius(i);
		contact_info::contact ci1, ci2;
		int cnt = compute_intersection(c, r, start, direction, ci1, &ci2);
		if (cnt == 0)
			continue;
		ci1.primitive_index = i;
		ci1.container = this;
		info.contacts.push_back(ci1);
		if (cnt == 1 || only_entry_points)
			continue;
		ci2.primitive_index = i;
		ci2.container = this;
		info.contacts.push_back(ci2);
	}
}

bool sphere_container::init(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, 1);
	cgv::render::ref_rounded_cone_renderer(ctx, 1);
	return true;
}

void sphere_container::clear(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, -1);
	cgv::render::ref_rounded_cone_renderer(ctx, -1);
}

void sphere_container::prepare_render(cgv::render::context& ctx, cgv::render::renderer& r, const cgv::render::render_style& rs, const std::vector<uint32_t>* indices_ptr) const
{
	primitive_container::prepare_render(ctx, r, rs, indices_ptr);
	auto& sr = reinterpret_cast<cgv::render::sphere_renderer&>(r);
	if (scaling_mode == SM_UNIFORM)
		sr.set_radius_array(ctx, uniform_scales);
}

void sphere_container::draw(cgv::render::context& ctx)
{
	switch (render_type) {
	case SRT_SPHERES: render(ctx, cgv::render::ref_sphere_renderer(ctx), srs); break;
	case SRT_ROUNDED_CONES: render(ctx, cgv::render::ref_rounded_cone_renderer(ctx), rcrs); break;
	case SRT_TUBES: render(ctx, cgv::render::ref_rounded_cone_renderer(ctx), rcrs, &indices); break;
	}
}
const cgv::render::render_style* sphere_container::get_render_style() const 
{
	if (render_type == SRT_SPHERES)
		return &srs;
	else
		return &rcrs;
}

	}
}