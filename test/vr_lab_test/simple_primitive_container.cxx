#include "simple_primitive_container.h"
#include <cgv/math/proximity.h>
#include <cgv/math/intersection.h>
#include <random>

simple_primitive_container::rgb simple_primitive_container::get_modified_color(const rgb& color) const
{
	rgb mod_col(color);
	switch (state) {
	case state_enum::grabbed:
		mod_col[1] = std::min(1.0f, mod_col[0] + 0.2f);
	case state_enum::close:
		mod_col[0] = std::min(1.0f, mod_col[0] + 0.2f);
		break;
	case state_enum::triggered:
		mod_col[1] = std::min(1.0f, mod_col[0] + 0.2f);
	case state_enum::pointed:
		mod_col[2] = std::min(1.0f, mod_col[2] + 0.2f);
		break;
	}
	return mod_col;
}
simple_primitive_container::simple_primitive_container(const std::string& _name, unsigned sphere_count)
	: cgv::nui::poseable(&active_position, name), active_position(nullptr)
{
	std::default_random_engine e;
	std::uniform_real_distribution<float> dr(0.6f, 1.3f);
	std::uniform_real_distribution<float> dc(-0.5f, 0.5f);
	for (unsigned i = 0; i < sphere_count; ++i) {
		positions.push_back(vec3(dc(e), dr(e), dc(e)));
		colors.push_back(rgb(dr(e), dr(e), dr(e)));
		radii.push_back(0.1f*dr(e));
	}
}

std::string simple_primitive_container::get_type_name() const
{
	return "simple_primitive_container";
}

void simple_primitive_container::on_set(void* member_ptr)
{
	if (member_ptr == &prim_idx) {
		active_position = &positions[prim_idx];
	}
	poseable::on_set(member_ptr);
}

bool simple_primitive_container::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx)
{
	float min_dist = std::numeric_limits<float>::max();
	vec3 q, n;
	for (size_t i = 0; i < positions.size(); ++i) {
		cgv::math::closest_point_on_sphere_to_point(positions[i], radii[i], point, q, n);
		float dist = (point - q).length();
		if (dist < min_dist) {
			primitive_idx = i;
			prj_point = q;
			prj_normal = n;
			min_dist = dist;
		}
	}
	//std::cout << "min_dist = " << positions[0] << " <-> " << point << " | " << radii[0] << " at " << min_dist << " for " << primitive_idx << std::endl;
	return min_dist < std::numeric_limits<float>::max();
}
bool simple_primitive_container::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	vec3 ro = ray_start;
	vec3 rd = ray_direction;
	hit_param = std::numeric_limits<float>::max();
	for (size_t i = 0; i < positions.size(); ++i) {
		vec2 res;
		if (cgv::math::ray_sphere_intersection(ro, rd, positions[i], radii[i], res) == 0)
			continue;
		float param;
		if (res[0] < 0) {
			if (res[1] < 0)
				continue;
			param = res[1];
		}
		else
			param = res[0];
		if (param < hit_param) {
			primitive_idx = i;
			hit_param = param;
			hit_normal = normalize(ro+param*rd-positions[i]);
		}
	}
	return hit_param < std::numeric_limits<float>::max();
}
bool simple_primitive_container::init(cgv::render::context& ctx)
{
	if (!poseable::init(ctx))
		return false;
	cgv::render::ref_sphere_renderer(ctx, 1);
	return true;
}
void simple_primitive_container::clear(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, -1);
	poseable::clear(ctx);
}
void simple_primitive_container::draw(cgv::render::context& ctx)
{
	poseable::draw(ctx);
	// show spheres
	auto& sr = cgv::render::ref_sphere_renderer(ctx);
	sr.set_render_style(srs);
	sr.set_position_array(ctx, positions);
	rgb tmp_color;
	if (prim_idx >= 0 && prim_idx < positions.size()) {
		tmp_color = colors[prim_idx];
		colors[prim_idx] = get_modified_color(colors[prim_idx]);
	}
	sr.set_color_array(ctx, colors);
	sr.set_radius_array(ctx, radii);
	sr.render(ctx, 0, positions.size());
	if (prim_idx >= 0 && prim_idx < positions.size())
		colors[prim_idx] = tmp_color;
}
void simple_primitive_container::create_gui()
{
	add_decorator(get_name(), "heading", "level=2");
	if (begin_tree_node("style", srs)) {
		align("\a");
		add_gui("srs", srs);
		align("\b");
		end_tree_node(srs);
	}
	poseable::create_gui();
}
