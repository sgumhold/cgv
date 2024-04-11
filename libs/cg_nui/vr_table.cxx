#include "vr_table.h"
#include "table_gizmo.h"
#include <cgv/math/intersection.h>
#include <cgv/math/ftransform.h>
#include <cg_vr/vr_events.h>
#include <random>

namespace cgv {
	namespace nui {

	mat4 vr_table::get_transform() const
	{
		return cgv::math::rotate4<float>(float(180/M_PI*angle), 0, 1, 0) *
			cgv::math::translate4<float>(0, height, 0) * 
			cgv::math::scale4<float>(0.71f*scale, 0.71f * scale, 0.71f * scale);
	}
	mat3 vr_table::get_normal_transform() const
	{
		mat4 M = cgv::math::scale4<float>(1 / (0.71f * scale), 1 / (0.71f * scale), 1 / (0.71f * scale))*
			     cgv::math::rotate4<float>(-float(180/M_PI*angle), 0, 1, 0);
		return mat3(4, 4, &M(0, 0));
	}
	void vr_table::construct_rectangular_table(float tw, float td, float th, float tW, float tpO, rgb table_clr, rgb leg_clr)
	{
		float tO = tpO * tw;
		float x0 = -0.5f * tw;
		float x1 = -0.5f * tw + tO;
		float x2 = -0.5f * tw + tO + tW;
		float x3 = 0.5f * tw - tO - tW;
		float x4 = 0.5f * tw - tO;
		float x5 = 0.5f * tw;
		float y0 = 0;
		float y1 = th - tW;
		float y2 = th;
		float z0 = -0.5f * td;
		float z1 = -0.5f * td + tO;
		float z2 = -0.5f * td + tO + tW;
		float z3 = 0.5f * td - tO - tW;
		float z4 = 0.5f * td - tO;
		float z5 = 0.5f * td;
		boxes.push_back(box3(vec3(x0, y1, z0), vec3(x5, y2, z5))); box_colors.push_back(table_clr);

		boxes.push_back(box3(vec3(x1, y0, z1), vec3(x2, y1, z2))); box_colors.push_back(leg_clr);
		boxes.push_back(box3(vec3(x3, y0, z1), vec3(x4, y1, z2))); box_colors.push_back(leg_clr);
		boxes.push_back(box3(vec3(x3, y0, z3), vec3(x4, y1, z4))); box_colors.push_back(leg_clr);
		boxes.push_back(box3(vec3(x1, y0, z3), vec3(x2, y1, z4))); box_colors.push_back(leg_clr);
	}
	void vr_table::construct_round_table(float ttr, float tbr, float th, float tW, float tpO, rgb table_clr, rgb leg_clr)
	{
		float r0 = 0.5f * tbr;
		float r1 = 0.5f * (1.0f - tpO) * tbr;
		float r2 = 2 * tW;
		float r3 = 2 * tW;
		float r4 = 0.5f * (1.0f - tpO) * ttr;
		float r5 = 0.5f * ttr;
		float y0 = 0;
		float y1 = tW;
		float y2 = 5 * tW;
		float y3 = th - 5 * tW;
		float y4 = th - tW;
		float y5 = th;

		cone_vertices.push_back(vec4(0, y0, 0, r0));
		cone_vertices.push_back(vec4(0, y1, 0, r0));

		cone_vertices.push_back(vec4(0, y1, 0, r1));
		cone_vertices.push_back(vec4(0, y2, 0, r2));

		cone_vertices.push_back(vec4(0, y2, 0, r2));
		cone_vertices.push_back(vec4(0, y3, 0, r3));

		cone_vertices.push_back(vec4(0, y3, 0, r3));
		cone_vertices.push_back(vec4(0, y4, 0, r4));
		cone_colors.push_back(table_clr);
		cone_colors.push_back(table_clr);
		for (size_t i = 2; i < cone_vertices.size(); ++i)
			cone_colors.push_back(leg_clr);
		cone_vertices.push_back(vec4(0, y4, 0, r5));
		cone_vertices.push_back(vec4(0, y5, 0, r5));
		cone_colors.push_back(table_clr);
		cone_colors.push_back(table_clr);
	}
	bool vr_table::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx)
	{
		prj_point = point;
		prj_point[1] = height;
		if (mode == table_mode::round) {
			float radius = sqrt(point[0] * point[0] + point[2] * point[2]);
			float f = 0.5f*scale / radius;
			prj_point[0] *= f;
			prj_point[2] *= f;
		}
		else {

		}
		prj_normal = vec3(0, 1, 0);
		return true;
	}
	bool vr_table::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
	{
		vec3 n;
		float t;
		//t = intersection::ray_cylinder_intersection(ray_start, ray_direction, vec3(0, height - leg_width, 0), vec3(0, leg_width, 0), 0.5f * scale, n);
		if (cgv::math::ray_cylinder_intersection({ ray_start, ray_direction }, vec3(0, height - leg_width, 0), vec3(0, leg_width, 0), 0.5f * scale, t, &n)) {
			hit_param = t;
			hit_normal = n;
			return true;
		}
		return false;
	}
	rgb vr_table::get_table_color() const
	{
		rgb color = table_color;
		static float rgb_off[15] = { 0,0,0, 0.1f,0.0f,0.1f, 0.0f,0.1f,0.1f, 0.2f,0.0f,0.2f, 0.0f,0.2f,0.2f };
		for (int i = 0; i < 2; ++i)
			color[i] = std::min(1.0f, color[i] + rgb_off[3*int(state)+i]);
		return color;
	}
	void vr_table::build_table()
	{
		boxes.clear();
		box_colors.clear();
		cone_vertices.clear();
		cone_colors.clear();
		rgb color = get_table_color();
		switch (mode) {
		case table_mode::rectangular:
			construct_rectangular_table(scale, scale / aspect, height, leg_width, percentual_leg_offset, color, leg_color);
			break;
		case table_mode::round:
			construct_round_table(scale, scale / aspect, height, leg_width, percentual_leg_offset, color, leg_color);
			break;
		default:
			break;
		}
	}
	void vr_table::update_table_color(const rgb& color)
	{
		switch (mode) {
		case table_mode::rectangular:
			box_colors.front() = color;
			break;
		case table_mode::round:
			cone_colors[0] = cone_colors[1] = cone_colors.back() = cone_colors[cone_colors.size() - 2] = color;
			break;
		default:
			break;
		}
	}
	vr_table::vr_table()
	{
		set_name("vr_table");
		table_color = rgb(0.3f, 0.2f, 0.0f);
		leg_color = rgb(0.2f, 0.1f, 0.1f);
		wire_color = rgb(0.5f, 0.5f, 0.5f);
		build_table();
		use_gizmo = false;
		gizmo = new table_gizmo();
		append_child(gizmo);
		//cgv::base::register_object(gizmo);
	}
	cgv::reflect::enum_reflection_traits<table_mode> get_reflection_traits(const table_mode& tm)
	{
		return cgv::reflect::enum_reflection_traits<table_mode>("rectangular,round");
	}
	bool vr_table::self_reflect(cgv::reflect::reflection_handler& rh)
	{
		return
			rh.reflect_member("mode", mode) &&
			rh.reflect_member("angle", angle) &&
			rh.reflect_member("color", table_color) &&
			rh.reflect_member("show_wirebox", show_wirebox) &&
			rh.reflect_member("wire_color", wire_color) &&
			rh.reflect_member("scale", scale) &&
			rh.reflect_member("aspect", aspect) &&
			rh.reflect_member("height", height) &&
			rh.reflect_member("leg_color", leg_color) &&
			rh.reflect_member("leg_width", leg_width) &&
			rh.reflect_member("percentual_leg_offset", percentual_leg_offset);
	}
	void vr_table::on_set(void* member_ptr)
	{
		if (member_ptr == &use_gizmo) {
			if (use_gizmo)
				gizmo->attach(this);
			else
				gizmo->detach();
		}
		if (member_ptr == &angle || member_ptr == &height || member_ptr == &scale) {
			if (use_gizmo)
				gizmo->on_set(member_ptr);
		}
		if (member_ptr >= &mode && member_ptr < &leg_color + 1) {
			build_table();
		}
		if (member_ptr == &state) {
			update_table_color(get_table_color());
		}
		update_member(member_ptr);
		post_redraw();
	}
	bool vr_table::init(cgv::render::context& ctx)
	{
		cgv::render::ref_box_renderer(ctx, 1);
		cgv::render::ref_box_wire_renderer(ctx, 1);
		return cone_renderer.init(ctx);
	}
	void vr_table::clear(cgv::render::context& ctx)
	{
		cgv::render::ref_box_renderer(ctx, -1);
		cgv::render::ref_box_wire_renderer(ctx, -1);
		cone_renderer.clear(ctx);
	}
	void vr_table::draw(cgv::render::context& ctx)
	{
		if (!boxes.empty()) {
			auto& br = cgv::render::ref_box_renderer(ctx);
			br.set_render_style(box_style);

			br.set_box_array(ctx, boxes);
			br.set_color_array(ctx, box_colors);
			br.render(ctx, 0, (GLsizei)boxes.size());
		}
		if (!cone_vertices.empty()) {
			cone_renderer.set_render_style(cone_style);
			cone_renderer.set_sphere_array(ctx, cone_vertices);
			cone_renderer.set_color_array(ctx, cone_colors);
			cone_renderer.render(ctx, 0, cone_vertices.size());
		}
		if (show_wirebox) {
			ctx.push_modelview_matrix();
			ctx.mul_modelview_matrix(get_transform());
			auto& bwr = cgv::render::ref_box_wire_renderer(ctx);
			bwr.set_render_style(box_wire_style);
			bwr.set_box(ctx, box3(vec3(-0.5f, 0, -0.5f), vec3(0.5f, 1, 0.5f)));
			bwr.set_color(ctx, wire_color);
			bwr.render(ctx, 0, 1);
			ctx.pop_modelview_matrix();
		}
	}
	bool vr_table::focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info)
	{
		switch (action) {
		case cgv::nui::focus_change_action::attach:
			if (state == state_enum::idle) {
				state = dis_info.mode == cgv::nui::dispatch_mode::proximity ? state_enum::close : state_enum::pointed;
				hid_id = dis_info.hid_id;
				on_set(&state);
				return true;
			}
			if (state == state_enum::gizmo && dis_info.mode == cgv::nui::dispatch_mode::pointing) {
				hid_id = dis_info.hid_id;
				return true;
			}
			return false;
		case cgv::nui::focus_change_action::detach:
			if (state != state_enum::idle && state != state_enum::gizmo && dis_info.hid_id == hid_id) {
				state = state_enum::idle;
				on_set(&state);
			}
		}
		return true;
	}
	void vr_table::stream_help(std::ostream& os)
	{
		os << "vr_table: grab with grip to change rotation, height and scale" << std::endl;
	}
	void vr_table::place_table(const vec3& p0, const vec3& p1)
	{
		float r0 = sqrt(p0[0] * p0[0] + p0[2] * p0[2]);
		float r1 = sqrt(p1[0] * p1[0] + p1[2] * p1[2]);
		float dh = p1[1] - p0[1];
		float ds = r1 / r0;
		float da = asin(cross(vec3(p0[0], 0, p0[2]), vec3(p1[0], 0, p1[2]))[1] / (r0 * r1));
		height = height_at_grab + dh;
		scale = scale_at_grab * ds;
		angle = angle_at_grab + da;
		on_set(&height);
		update_member(&scale);
		update_member(&angle);
	}
	bool vr_table::handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request)
	{
		// only process events while in focus and only of focus hid_id
		if (state == state_enum::idle)
			return false;
		if (!(dis_info.hid_id == hid_id))
			return false;
		if (dis_info.mode == cgv::nui::dispatch_mode::structural)
			return false;

		bool activate;
		switch (state) {
		case state_enum::close:
			if (is_grab_change(e, activate) && activate) {
				scale_at_grab = scale;
				height_at_grab = height;
				angle_at_grab = angle;
				state = state_enum::grabbed;
				on_set(&state);
				// change focus to restrict to table asjustments
				drag_begin(request, false, original_config);
				return true;
			}
			if (is_grabbing(e, dis_info)) {
				query_point_at_grab = reinterpret_cast<const cgv::nui::proximity_dispatch_info&>(dis_info).query_point;
				return true;
			}
			return false;
		case state_enum::grabbed:
			if (is_grab_change(e, activate) && !activate) {
				state = state_enum::close;
				on_set(&state);
				// recover focus configuration
				drag_end(request, original_config);
				return true;
			}
			if (is_grabbing(e, dis_info)) {
				place_table(query_point_at_grab, reinterpret_cast<const cgv::nui::proximity_dispatch_info&>(dis_info).query_point);
				return true;
			}
			return false;
		case state_enum::pointed:
			if (is_trigger_change(e, activate) && activate) {
				use_gizmo = true;
				on_set(&use_gizmo);
				state = state_enum::gizmo;
				on_set(&state);
			}
			return true;
		case state_enum::gizmo:
			if (is_trigger_change(e, activate) && activate) {
				use_gizmo = false;
				on_set(&use_gizmo);
				state = state_enum::pointed;
				on_set(&state);
			}
			return true;
		}
		return false;
	}
	void vr_table::create_gui()
	{
		add_decorator("vr_table", "heading");
		add_member_control(this, "use_gizmo", use_gizmo, "toggle");
		add_member_control(this, "show_wirebox", show_wirebox, "check");
		add_member_control(this, "wire_color", wire_color);
		add_member_control(this, "state", state, "dropdown", "enums='idle,close,pointed,grabbed,gizmo'");
		add_member_control(this, "table", mode, "dropdown", "enums='rectangular,round'");
		add_member_control(this, "color", table_color);
		add_member_control(this, "scale", scale, "value_slider", "min=0.1;max=3.0;log=true;ticks=true");
		add_member_control(this, "height", height, "value_slider", "min=0.1;max=3.0;ticks=true");
		add_member_control(this, "angle", angle, "value_slider", "min=-3.5;max=3.5;ticks=true");
		add_member_control(this, "aspect", aspect, "value_slider", "min=0.333;max=3.0;log=true;ticks=true");
		add_member_control(this, "leg color", leg_color);
		add_member_control(this, "legs", leg_width, "value_slider", "min=0.0;max=0.3;ticks=true");
		add_member_control(this, "percentual_offset", percentual_leg_offset, "value_slider", "min=0.0;max=0.5;ticks=true");
		if (begin_tree_node("gizmo", gizmo)) {
			align("\a");
			inline_object_gui(gizmo);
			align("\b");
			end_tree_node(gizmo);
		}

		if (begin_tree_node("styles", table_color)) {
			if (begin_tree_node("boxes", box_style)) {
				align("\a");
				add_gui("box_style", box_style);
				align("\b");
				end_tree_node(box_style);
			}
			if (begin_tree_node("wire", box_wire_style)) {
				align("\a");
				add_gui("box_wire_style", box_wire_style);
				align("\b");
				end_tree_node(box_wire_style);
			}
			if (begin_tree_node("cones", cone_style)) {
				align("\a");
				add_gui("cone_style", cone_style);
				align("\b");
				end_tree_node(cone_style);
			}
			align("\b");
			end_tree_node(table_color);
		}
	}
	}
}