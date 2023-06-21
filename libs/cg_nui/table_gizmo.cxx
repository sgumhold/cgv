#include "table_gizmo.h"
#include <cgv/math/intersection.h>
#include <cgv/math/proximity.h>
#include <cgv/math/pose.h>
#include <cg_vr/vr_events.h>
#include <random>

#define debug_gizmo false

namespace cgv {
	namespace nui {
void table_gizmo::compute_arrow_geometry()
{
	arrow_positions.clear();
	arrow_directions.clear();
	arrow_colors.clear();

	vec3 origin = vec3(0, table->height, 0);
	float r = 0.5f * table->scale;
	float v = iis[1].in_focus ? 0.2f : 0;
	arrow_positions.push_back(origin);
	arrow_directions.push_back(vec3(0, 1.5f * r, 0));
	arrow_colors.push_back(rgb(v + 0.2f, v + 0.8f, v + 0.2f));

	for (unsigned i = 0; i < nr_scale_arrows; ++i) {
		float v = i == scale_arrow_focus_idx ? 0.2f : 0;
		float a = float(2 * M_PI * i * (1.0f / nr_scale_arrows));
		float c = cos(a);
		float s = sin(a);
		arrow_directions.push_back(0.5f * vec3(c, 0, s));
		arrow_positions.push_back(origin + r * vec3(c, 0, s));
		arrow_colors.push_back(rgb(v + 0.8f, v + 0.2f, v + 0.2f));
	}
}
void table_gizmo::compute_spline_geometry(bool in_focus)
{
	spline_positions.clear();
	spline_tangents.clear();
	vec3 origin = vec3(0, table->height, 0);
	float r = 0.5f * table->scale;
	float v = in_focus ? 0.2f : 0;
	for (unsigned i = 0; i <= nr_spline_segments; ++i) {
		float a = float(2 * M_PI * i * (1.0f / nr_spline_segments));
		float c = cos(a);
		float s = sin(a);
		spline_tangents.push_back(6.6f * r / nr_spline_segments * vec4(-s, 0, c, 0));
		spline_positions.push_back(origin + r * vec3(c, 0, s));
	}
	strs.surface_color = rgb(v + 0.2f, v + 0.2f, v + 0.8f);
}
table_gizmo::table_gizmo()
{
	set_name("table_gizmo");
	iis.resize(3);
	ars.radius_relative_to_length = 0;
	srs.radius = 0.02f;
	on_set(&radius);
	hide();
}
void table_gizmo::attach(vr_table_ptr _table)
{
	table = _table;
	compute_arrow_geometry();
	compute_spline_geometry(false);
	show();
}
void table_gizmo::detach()
{
	for (int idx = 0; idx < 3; ++idx) {
		if (iis[idx].in_focus && iis[idx].is_triggered) {
			if (debug_gizmo) {
				std::cout << "gizmo(" << idx << "):";
				if (iis[idx].hid_id.category == cgv::nui::hid_category::controller)
					std::cout << "ctrl[" << iis[idx].hid_id.index << "]";
				else
					std::cout << "mouse";
				std::cout << " - recover" << std::endl;
			}
			drag_end(cgv::nui::focus_attachment(iis[idx].hid_id), this, iis[idx].original_config);
			iis[idx].is_triggered = false;
		}
		iis[idx].in_focus = false;
	}
	scale_arrow_focus_idx = -1;
	hide();
	arrow_positions.clear();
	arrow_directions.clear();
	arrow_colors.clear();
	spline_positions.clear();
	spline_tangents.clear();
}
void table_gizmo::update_rotation_angle(float& v, float v0, const vec3& origin, const vec3& p0, const vec3& ro, const vec3& rd) const
{
	vec3 q;
	if (cgv::math::closest_point_on_line_to_circle(ro, rd, origin, vec3(0, 1, 0), 0.5f * table->scale, q) == std::numeric_limits<float>::max())
		return;
	//debug_points.push_back(q);
	float s = cross(p0 - origin, q - origin)[1];
	float c = dot(p0 - origin, q - origin);
	float da = atan2(s, c);
	v = v0 + da;
}
void table_gizmo::update_height(float& v, float v0, const vec3& p0, const vec3& ro, const vec3& rd) const
{
	vec3 q;
	if (!cgv::math::closest_point_on_line_to_line(p0, vec3(0, 1, 0), ro, rd, q))
		return;
	float dh = (q - p0)[1];
	v = v0 + dh;
	if (v < 0.1f)
		v = 0.1f;
	if (v > 2.0f)
		v = 2.0f;
}
void table_gizmo::update_scale(float& v, float v0, const vec3& p0, const vec3& ad, const vec3& ro, const vec3& rd) const
{
	vec3 q;
	if (!cgv::math::closest_point_on_line_to_line(p0, ad, ro, rd, q))
		return;
	float ds = dot(q - p0, ad / ad.length());
	v = v0 + 2 * ds;
	if (v < 0.5f)
		v = 0.5f;
	if (v > 4.0f)
		v = 4.0f;
}
bool table_gizmo::self_reflect(cgv::reflect::reflection_handler& rh)
{
	return rh.reflect_member("radius", radius);
}
void table_gizmo::on_set(void* member_ptr)
{
	if (member_ptr == &table->height || member_ptr == &table->angle || member_ptr == &table->scale) {
		compute_spline_geometry(iis[0].in_focus);
		compute_arrow_geometry();
	}
	if (member_ptr == &nr_scale_arrows) {
		compute_arrow_geometry();
	}
	if (member_ptr == &nr_spline_segments) {
		compute_spline_geometry(iis[0].in_focus);
	}
	if (member_ptr == &radius || member_ptr == &draw_radius_scale) {
		ars.radius_lower_bound = draw_radius_scale * radius;
		strs.radius = draw_radius_scale * radius;
	}
	update_member(member_ptr);
	post_redraw();
}
bool table_gizmo::focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info)
{
	// allow one focus per controlled variable
	if (action == cgv::nui::focus_change_action::attach) {
		size_t idx = static_cast<const cgv::nui::hit_dispatch_info&>(dis_info).get_hit_info()->primitive_index;
		auto& is = iis[idx > 2 ? 2 : idx];
		// refuse focus, if ctrl is already focused by other hid
		if (is.in_focus)
			return false;
		is.in_focus = true;
		update_member(&is.in_focus);
		is.hid_id = dis_info.hid_id;
		if (idx >= 2) {
			scale_arrow_focus_idx = int(idx - 2);
			update_member(&scale_arrow_focus_idx);
		}
		if (idx == 0)
			compute_spline_geometry(true);
		else
			compute_arrow_geometry();
		if (debug_gizmo) {
			std::cout << "gizmo(" << idx << "):";
			if (dis_info.hid_id.category == cgv::nui::hid_category::controller)
				std::cout << "ctrl[" << dis_info.hid_id.index << "]";
			else
				std::cout << "mouse";
			std::cout << " - attach" << std::endl;
		}
		post_redraw();
		return true;
	}
	if (action == cgv::nui::focus_change_action::index_change) {
		// first find old ctrl idx
		size_t old_idx;
		for (old_idx = 0; old_idx < 3; ++old_idx) {
			auto& is = iis[old_idx];
			if (is.hid_id == dis_info.hid_id)
				break;
		}
		// next extract new primitive and new ctrl indices
		size_t new_prim_idx = static_cast<const cgv::nui::hit_dispatch_info&>(dis_info).get_hit_info()->primitive_index;
		size_t new_idx = std::min(size_t(2), new_prim_idx);
		// if old ctrl index found
		if (old_idx < 3) {
			auto& ois = iis[old_idx];
			// for same ctrl indices
			if (new_idx == old_idx) {
				// only in case of multiple primitves an index change can happen
				if (new_idx != 2) {
					std::cerr << "index_change focus change action arised with identical indices" << std::endl;
					//abort();
				}
				// is_triggered status is not transmitted to new primitive in order to force fresh triggering
				ois.is_triggered = false;
				update_member(&ois.is_triggered);
				if (debug_gizmo) {
					std::cout << "gizmo(" << old_idx << "):";
					if (dis_info.hid_id.category == cgv::nui::hid_category::controller)
						std::cout << "ctrl[" << dis_info.hid_id.index << "]";
					else
						std::cout << "mouse";
					std::cout << " - index_change " << scale_arrow_focus_idx << " -> " << int(new_prim_idx - 2) << std::endl;
				}
				// update index of arrow in focus
				scale_arrow_focus_idx = int(new_prim_idx - 2);
				update_member(&scale_arrow_focus_idx);

				post_redraw();
				return true;
			}
			// for different ctrl indices

			// first turn off focus in old ctrl
			ois.in_focus = false;
			ois.is_triggered = false;
			update_member(&ois.in_focus);
			update_member(&ois.is_triggered);
			if (old_idx == 2) {
				scale_arrow_focus_idx = -1;
				update_member(&scale_arrow_focus_idx);
			}
			if (debug_gizmo) {
				std::cout << "gizmo(" << old_idx << "):";
				if (dis_info.hid_id.category == cgv::nui::hid_category::controller)
					std::cout << "ctrl[" << dis_info.hid_id.index << "]";
				else
					std::cout << "mouse";
				std::cout << " - index_change - discard " << old_idx << " <";
				if (ois.hid_id.category == cgv::nui::hid_category::controller)
					std::cout << "ctrl[" << ois.hid_id.index << "]";
				else
					std::cout << "mouse";
				std::cout << ">" << std::endl;
			}
			ois.hid_id = cgv::nui::hid_identifier();
		}
		// next or if old ctrl was not found ensure that new ctrl is not already in focus by other hid
		auto& nis = iis[new_idx];
		if (nis.in_focus) {
			post_redraw();
			return false;
		}
		nis.in_focus = true;
		update_member(&nis.in_focus);
		nis.hid_id = dis_info.hid_id;
		if (new_idx == 2) {
			scale_arrow_focus_idx = int(new_prim_idx - 2);
			update_member(&scale_arrow_focus_idx);
		}

		if (debug_gizmo) {
			std::cout << "gizmo(" << old_idx << "):";
			if (dis_info.hid_id.category == cgv::nui::hid_category::controller)
				std::cout << "ctrl[" << dis_info.hid_id.index << "]";
			else
				std::cout << "mouse";
			std::cout << " - index_change - attach " << new_idx << " <";
			if (nis.hid_id.category == cgv::nui::hid_category::controller)
				std::cout << "ctrl[" << nis.hid_id.index << "]";
			else
				std::cout << "mouse";
			std::cout << ">" << std::endl;
		}
		// finally update all geometry
		compute_spline_geometry(iis[0].in_focus);
		compute_arrow_geometry();
	}
	else if (action == cgv::nui::focus_change_action::detach) {
		for (size_t idx = 0; idx < 3; ++idx) {
			auto& is = iis[idx];
			if (is.hid_id == dis_info.hid_id) {
				is.in_focus = false;
				is.is_triggered = false;
				update_member(&is.in_focus);
				update_member(&is.is_triggered);
				if (idx == 2) {
					scale_arrow_focus_idx = -1;
					update_member(&scale_arrow_focus_idx);
				}
				is.hid_id = cgv::nui::hid_identifier();
				if (debug_gizmo) {
					std::cout << "gizmo(" << idx << "):";
					if (dis_info.hid_id.category == cgv::nui::hid_category::controller)
						std::cout << "ctrl[" << dis_info.hid_id.index << "]";
					else
						std::cout << "mouse";
					std::cout << " - detach" << std::endl;
				}
				if (idx == 0)
					compute_spline_geometry(false);
				else
					compute_arrow_geometry();
			}
		}
	}
	post_redraw();
	return true;
}
void table_gizmo::stream_help(std::ostream& os)
{
	os << "table_gizmo: drag arrows or tube to scale, change height or rotate" << std::endl;
}
bool table_gizmo::handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request)
{
	// get index of interaction handle
	int idx;
	for (idx = 0; idx < 3; ++idx)
		if (iis[idx].hid_id == dis_info.hid_id)
			break;
	if (idx == 3)
		return false;
	if (!iis[idx].in_focus)
		return false;
	// check for vr key events
	float* value_ptrs[3] = { &table->angle, &table->height, &table->scale };
	bool triggered;
	if (is_trigger_change(e, triggered) && triggered != iis[idx].is_triggered) {
		if (triggered) {
			iis[idx].is_triggered = true;
			update_member(&iis[idx].is_triggered);
			iis[idx].value_at_trigger = *(value_ptrs[idx]);
			if (idx == 2 && dis_info.mode == cgv::nui::dispatch_mode::pointing) {
				const auto& inter_dis_info = reinterpret_cast<const cgv::nui::intersection_dispatch_info&>(dis_info);
				iis[idx].arrow_index = int(inter_dis_info.primitive_index) - 2;
			}
			drag_begin(request, true, iis[idx].original_config);
			if (debug_gizmo) {
				std::cout << "gizmo(" << idx << "):";
				if (dis_info.hid_id.category == cgv::nui::hid_category::controller)
					std::cout << "ctrl[" << dis_info.hid_id.index << "]";
				else
					std::cout << "mouse";
				std::cout << " - reconfigure" << std::endl;
			}
			return true;
		}
		else {
			iis[idx].is_triggered = false;
			update_member(&iis[idx].is_triggered);
			if (debug_gizmo) {
				std::cout << "gizmo(" << idx << "):";
				if (dis_info.hid_id.category == cgv::nui::hid_category::controller)
					std::cout << "ctrl[" << dis_info.hid_id.index << "]";
				else
					std::cout << "mouse";
				std::cout << " - recover" << std::endl;
			}
			drag_end(request, iis[idx].original_config);
			return true;
		}
	}
	if (is_pointing(e, dis_info)) {
		const auto& inter_dis_info = reinterpret_cast<const cgv::nui::intersection_dispatch_info&>(dis_info);
		if (!iis[idx].is_triggered) {
			if (inter_dis_info.ray_param < std::numeric_limits<float>::max())
				iis[idx].hit_point_at_trigger = inter_dis_info.hit_point;
		}
		else {
			switch (idx) {
			case 0:
				//debug_points.clear();
				//debug_points.push_back(iis[idx].hit_point_at_trigger);
				update_rotation_angle(table->angle, iis[idx].value_at_trigger, vec3(0, table->height, 0), iis[idx].hit_point_at_trigger, inter_dis_info.ray_origin, inter_dis_info.ray_direction);
				table->on_set(&table->angle);
				break;
			case 1:
				update_height(table->height, iis[idx].value_at_trigger, iis[idx].hit_point_at_trigger, inter_dis_info.ray_origin, inter_dis_info.ray_direction);
				table->on_set(&table->height);
				break;
			case 2:
				update_scale(table->scale, iis[idx].value_at_trigger, iis[idx].hit_point_at_trigger, arrow_directions[iis[idx].arrow_index + 1], inter_dis_info.ray_origin, inter_dis_info.ray_direction);
				table->on_set(&table->scale);
				break;
			}
		}
		return true;
	}
	return false;
}
bool table_gizmo::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx)
{
	if (!is_visible() || !table)
		return false;

	vec3 origin = vec3(0, table->height, 0);
	float r = 0.5f * table->scale;

	// first check arrows
	size_t idx = -1;
	vec3 p, n;
	float dist_min = std::numeric_limits<float>::max();
	for (size_t i = 0; i < arrow_positions.size(); ++i) {
		vec3 p1, n1;
		cgv::math::closest_point_on_cylinder_to_point(arrow_positions[i], arrow_directions[i], radius, point, p1, n1);
		float dist = (p1 - point).length();
		if (dist < dist_min) {
			dist_min = dist;
			p = p1;
			n = n1;
			idx = i + 1;
		}
	}
	// then check torus
	vec3 d_xz = point - origin;
	d_xz.y() = 0;

	vec3 q0;
	float r0 = d_xz.length();
	if (r0 < 1e-6)
		q0 = origin + (vec3(r, 0, 0));
	else
		q0 = origin + (r / r0) * d_xz;
	vec3 d_ry = point - q0;
	float r1 = d_ry.length();
	vec3 q1;
	vec3 n1;
	if (r1 < 1e-6) {
		q1 = q0 + (radius / r0) * d_xz;
		n1 = (1 / r0) * d_xz;
	}
	else {
		q1 = q0 + (radius / r1) * d_ry;
		n1 = (1 / r1) * d_ry;
	}
	float dist = (q1 - point).length();
	if (dist < dist_min) {
		p = q1;
		n = n1;
		dist_min = dist;
		idx = 0;
	}
	prj_point = p;
	prj_normal = n;
	primitive_idx = idx;
	return true;
}
bool table_gizmo::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	if (!is_visible() || !table)
		return false;

	vec3 origin = vec3(0, table->height, 0);
	float r = 0.5f * table->scale;

	// first check arrows
	size_t idx = -1;
	float t = std::numeric_limits<float>::max();
	vec3 n;
	for (size_t i = 0; i < arrow_positions.size(); ++i) {
		vec3 n0;
		float t0;
		if(cgv::math::ray_cylinder_intersection({ ray_start, ray_direction }, arrow_positions[i], arrow_directions[i], radius, t0, &n0) && t0 < t) {
			t = t0;
			n = n0;
			idx = i + 1;
		}
	}
	// then check torus
	vec3 n0;
	float t0;
	if (cgv::math::ray_torus_intersection({ ray_start, ray_direction }, origin, vec3(0, 1, 0), r, radius, t0, &n0) && t0 < t) {
		t = t0;
		n = n0;
		idx = 0;
	}
	if (t == std::numeric_limits<float>::max())
		return false;
	hit_param = t;
	hit_normal = n;
	primitive_idx = idx;
	return true;
}
bool table_gizmo::init(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, 1);
	cgv::render::ref_arrow_renderer(ctx, 1);
	cgv::render::ref_spline_tube_renderer(ctx, 1);
	return true;
}
void table_gizmo::clear(cgv::render::context& ctx)
{
	cgv::render::ref_sphere_renderer(ctx, -1);
	cgv::render::ref_arrow_renderer(ctx, -1);
	cgv::render::ref_spline_tube_renderer(ctx, -1);
}
void table_gizmo::draw(cgv::render::context& ctx)
{
	if (!debug_points.empty()) {
		auto& sr = cgv::render::ref_sphere_renderer(ctx);
		sr.set_render_style(srs);
		sr.set_position_array(ctx, debug_points);
		sr.render(ctx, 0, debug_points.size());
	}
	if (!arrow_positions.empty()) {
		auto& ar = cgv::render::ref_arrow_renderer(ctx);
		ar.set_render_style(ars);

		ar.set_position_array(ctx, arrow_positions);
		ar.set_direction_array(ctx, arrow_directions);
		ar.set_color_array(ctx, arrow_colors);
		ar.render(ctx, 0, (GLsizei)arrow_positions.size());
	}
	if (!spline_positions.empty()) {
		auto& str = cgv::render::ref_spline_tube_renderer(ctx);
		str.set_render_style(strs);
		str.set_position_array(ctx, spline_positions);
		str.set_tangent_array(ctx, spline_tangents);
		str.render(ctx, 0, spline_positions.size(), true);
	}
}
void table_gizmo::create_gui()
{
	add_decorator("table_gizmo", "heading");
	add_member_control(this, "radius", radius, "value_slider", "min=0.01;max=0.2;log=true;ticks=true");
	add_member_control(this, "draw_radius_scale", draw_radius_scale, "value_slider", "min=0.1;max=2;log=true;ticks=true");
	add_member_control(this, "nr_spline_segments", nr_spline_segments, "value_slider", "min=2;max=16;log=true;ticks=true");
	add_member_control(this, "nr_scale_arrows", nr_scale_arrows, "value_slider", "min=2;max=16;log=true;ticks=true");
	add_member_control(this, "spline_color", strs.surface_color);
	add_member_control(this, "in_focus[0]", iis[0].in_focus, "toggle");
	add_member_control(this, "is_triggered[0]", iis[0].is_triggered, "toggle");
	add_member_control(this, "in_focus[1]", iis[1].in_focus, "toggle");
	add_member_control(this, "is_triggered[1]", iis[1].is_triggered, "toggle");
	add_member_control(this, "in_focus[2]", iis[2].in_focus, "toggle");
	add_member_control(this, "is_triggered[2]", iis[2].is_triggered, "toggle");
	if (begin_tree_node("styles", nr_spline_segments)) {
		if (begin_tree_node("spheres", srs)) {
			align("\a");
			add_gui("srs", srs);
			align("\b");
			end_tree_node(srs);
		}
		if (begin_tree_node("arrows", ars)) {
			align("\a");
			add_gui("ars", ars);
			align("\b");
			end_tree_node(ars);
		}
		if (begin_tree_node("tubes", strs)) {
			align("\a");
			add_gui("strs", strs);
			align("\b");
			end_tree_node(strs);
		}
		end_tree_node(nr_spline_segments);
	}
}
	}
}