#include "sphere_container.h"

namespace cgv {
	namespace nui {

sphere_container::sphere_container(nui_node* _parent, bool use_radii, bool _use_colors, SphereRenderType _render_type)
	: primitive_container(_parent, PT_SPHERE, _use_colors, false, use_radii?SM_UNIFORM:SM_NONE), render_type(_render_type)
{
	srs.radius = 1.0f;
	srs.radius_scale = 1.0f;
}

std::string sphere_container::get_primitive_type() const
{
	return "sphere";
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

sphere_container::box3 sphere_container::get_oriented_bounding_box(uint32_t i) const
{
	const vec3& c = center_positions[i];
	float r = get_radius(i);
	return box3(c - r, c + r);
}

bool sphere_container::compute_closest_point(contact_info& info, const vec3& pos)
{
	bool result = false;
	contact_info::contact C;
	C.container = this;
	C.node = get_parent();
	for (const auto& c : center_positions) {
		C.primitive_index = uint32_t(&c - &center_positions.front());
		float r = get_radius(C.primitive_index);
		C.normal = normalize(pos - c);
		C.position = c + r*C.normal;
		C.distance = (C.position - pos).length();
		if (info.consider_closest_contact(C))
			result = true;
	}
	return result;
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
	C.normal = C.texcoord = normalize(C.position - ce);
	if (C2_ptr) {
		C2_ptr->distance = -b + h;
		C2_ptr->position = ro + C2_ptr->distance * rd;
		C2_ptr->normal = C2_ptr->texcoord = normalize(C2_ptr->position - ce);
	}
	return 2;
}

bool sphere_container::compute_first_intersection(contact_info& info, const vec3& start, const vec3& direction)
{
	bool result = false;
	contact_info::contact C;
	C.container = this;
	C.node = get_parent();
	for (const auto& c : center_positions) {
		C.primitive_index = uint32_t(&c - &center_positions.front());
		float r = get_radius(C.primitive_index);
		if (compute_intersection(c, r, start, direction, C) > 0) {
			if (info.consider_closest_contact(C))
				result = true;
		}
	}
	return result;
}

int sphere_container::compute_all_intersections(contact_info& info, const vec3& start, const vec3& direction, bool only_entry_points)
{
	int result = 0;
	contact_info::contact C1, C2;
	C1.container = C2.container = this;
	C1.node = C2.node = get_parent();
	for (const auto& c : center_positions) {
		C1.primitive_index = C2.primitive_index = uint32_t(&c - &center_positions.front());
		float r = get_radius(C1.primitive_index);
		int nr_intersections = compute_intersection(c, r, start, direction, C1, &C2);
		if (nr_intersections == 0)
			continue;
		++result;
		info.contacts.push_back(C1);
		if (nr_intersections == 1 || only_entry_points)
			continue;
		++result;
		info.contacts.push_back(C2);
	}
	return result;
}

bool sphere_container::init(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, 1);
	cgv::render::ref_cone_renderer(ctx, 1);
	return true;
}

void sphere_container::clear(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, -1);
	cgv::render::ref_cone_renderer(ctx, -1);
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
	case SRT_ROUNDED_CONES: render(ctx, cgv::render::ref_cone_renderer(ctx), rcrs); break;
	case SRT_TUBES: render(ctx, cgv::render::ref_cone_renderer(ctx), rcrs, &indices); break;
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