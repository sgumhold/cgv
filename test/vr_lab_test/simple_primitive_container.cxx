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
	: poseable(name), translatable()
{
	std::default_random_engine e;
	std::uniform_real_distribution<float> dr(0.6f, 1.3f);
	std::uniform_real_distribution<float> dc(-0.5f, 0.5f);
	for (unsigned i = 0; i < sphere_count; ++i) {
		vec3 position = vec3(dc(e), dr(e), dc(e));
		float radius = 0.1f * dr(e);
		positions.push_back(position);
		radii.push_back(radius);
		colors.push_back(rgb(dr(e), dr(e), dr(e)));
	}

	active_gizmo_ui = active_gizmo;

	trans_gizmo = new cgv::nui::translation_gizmo();
	trans_gizmo->set_anchor_offset_position(&active_position_ptr);
	trans_gizmo->set_position_reference(this);
	trans_gizmo->set_axes_directions({ vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) });
	trans_gizmo->set_axes_positions(
		{ vec3(0.5f, 0.0f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 0.0f, 0.5f) },
		{ vec3(0.02f, 0.0f, 0.0f), vec3(0.0f, 0.02f, 0.0f), vec3(0.0f, 0.0f, 0.02f) }
	);
	if (active_gizmo == ActiveGizmoOptions::AGO_TRANSLATION)
		trans_gizmo->attach();
}

std::string simple_primitive_container::get_type_name() const
{
	return "simple_primitive_container";
}

void simple_primitive_container::on_set(void* member_ptr)
{
	if (member_ptr == &prim_idx) {
		active_position_ptr = &positions[prim_idx];
	}

	if (member_ptr == &active_gizmo_ui)
	{
		if (active_gizmo_ui != active_gizmo) {
			switch (active_gizmo) {
			case ActiveGizmoOptions::AGO_TRANSLATION: trans_gizmo->detach(); break;
			case ActiveGizmoOptions::AGO_ROTATION: rot_gizmo->detach(); break;
			case ActiveGizmoOptions::AGO_SCALING: scale_gizmo->detach(); break;
			}
			switch (active_gizmo_ui) {
			case ActiveGizmoOptions::AGO_TRANSLATION: trans_gizmo->attach(); break;
				//case ActiveGizmoOptions::AGO_ROTATION: rot_gizmo->attach(this, &position, &rotation, &extent); break;
				//case ActiveGizmoOptions::AGO_SCALING: scale_gizmo->attach(this, &extent, &position, &rotation); break;
			}
			active_gizmo = active_gizmo_ui;
		}
	}
	
	update_member(member_ptr);
	post_redraw();
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
	return min_dist < std::numeric_limits<float>::max();
}
bool simple_primitive_container::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	vec3 ro = ray_start;
	vec3 rd = ray_direction;
	hit_param = std::numeric_limits<float>::max();
	for (size_t i = 0; i < positions.size(); ++i) {
		//vec2 res;
		//if (cgv::math::ray_sphere_intersection(ro, rd, positions[i], radii[i], res) == 0)
		//	continue;
		//float param;
		//if (res[0] < 0) {
		//	if (res[1] < 0)
		//		continue;
		//	param = res[1];
		//}
		//else
		//	param = res[0];
		//if (param < hit_param) {
		//	primitive_idx = i;
		//	hit_param = param;
		//	hit_normal = normalize(ro+param*rd-positions[i]);
		//}
	
		vec3 normal;
		float param;
		int n_intersections = cgv::math::first_ray_sphere_intersection(cgv::math::ray<float, 3>(ro, rd), positions[i], radii[i], param, &normal);
		if (n_intersections > 0 && param < hit_param) {
			primitive_idx = i;
			hit_param = param;
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
	add_member_control(this, "Active Gizmo", active_gizmo_ui, "dropdown", "enums='None,Translation,Scaling,Rotation'");
	if (begin_tree_node("style", srs)) {
		align("\a");
		add_gui("srs", srs);
		align("\b");
		end_tree_node(srs);
	}
	poseable::create_gui();
}

cgv::render::render_types::vec3 simple_primitive_container::get_position() const
{
	return positions[prim_idx];
}

void simple_primitive_container::set_position(const vec3& position)
{
	positions[prim_idx] = position;
}
