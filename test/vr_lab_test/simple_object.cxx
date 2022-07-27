#include "simple_object.h"
#include <cgv/math/proximity.h>
#include <cgv/math/intersection.h>

cgv::render::shader_program simple_object::prog;

simple_object::rgb simple_object::get_modified_color(const rgb& color) const
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

simple_object::simple_object(const std::string& _name, const vec3& _position, const rgb& _color, const vec3& _extent, const quat& _rotation) :
	cgv::nui::poseable(&position, &rotation, _name), position(_position), rotation(_rotation),
	color(_color), extent(_extent)
{
	name = _name;

	//debug_point = position + 0.5f*extent;
	brs.rounding = true;
	brs.default_radius = 0.02f;

	active_gizmo_ui = active_gizmo;

	trans_gizmo = new cgv::nui::translation_gizmo();
	trans_gizmo->set_anchor_object(this);
	trans_gizmo->set_position_reference(this);
	trans_gizmo->configure_axes_directions({ vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) });
	trans_gizmo->configure_axes_positioning(
		{ vec3(0.5f, 0.0f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 0.0f, 0.5f) },
		{ vec3(0.02f, 0.0f, 0.0f), vec3(0.0f, 0.02f, 0.0f), vec3(0.0f, 0.0f, 0.02f) }
	);
	if (active_gizmo == ActiveGizmoOptions::AGO_TRANSLATION)
		trans_gizmo->attach();

	//rot_gizmo = new cgv::nui::rotation_gizmo();
	//append_child(rot_gizmo);
	//rot_gizmo->configure_axes_directions({ vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) });
	//if (active_gizmo == ActiveGizmoOptions::AGO_ROTATION)
	//	rot_gizmo->attach(this, &position, &rotation, &extent);
	//
	//scale_gizmo = new cgv::nui::scaling_gizmo();
	//append_child(scale_gizmo);
	//scale_gizmo->configure_axes_directions({ vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) });
	//scale_gizmo->configure_axes_positioning(
	//	{ vec3(0.5f, 0.0f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 0.0f, 0.5f) },
	//	{ vec3(0.02f, 0.0f, 0.0f), vec3(0.0f, 0.02f, 0.0f), vec3(0.0f, 0.0f, 0.02f) }
	//);
	//if (active_gizmo == ActiveGizmoOptions::AGO_SCALING)
	//	scale_gizmo->attach(this, &extent, &position, &rotation);
}

std::string simple_object::get_type_name() const
{
	return "simple_object";
}

bool simple_object::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx)
{
	vec3 p = point - position;
	rotation.inverse_rotate(p);
	for (int i = 0; i < 3; ++i)
		p[i] = std::max(-0.5f * extent[i], std::min(0.5f * extent[i], p[i]));
	rotation.rotate(p);
	prj_point = p + position;
	return true;
}

bool simple_object::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	vec3 ro = ray_start - position;
	vec3 rd = ray_direction;
	rotation.inverse_rotate(ro);
	rotation.inverse_rotate(rd);
	vec3 n;
	vec2 res;
	if (cgv::math::ray_box_intersection(ro, rd, 0.5f*extent, res, n) == 0)
		return false;
	if (res[0] < 0) {
		if (res[1] < 0)
			return false;
		hit_param = res[1];
	}
	else {
		hit_param = res[0];
	}
	rotation.rotate(n);
	hit_normal = n;
	return true;
}

bool simple_object::init(cgv::render::context& ctx)
{
	poseable::init(ctx);
	auto& br = cgv::render::ref_box_renderer(ctx, 1);
	if (prog.is_linked())
		return true;
	return br.build_program(ctx, prog, brs);
}
void simple_object::clear(cgv::render::context& ctx)
{
	cgv::render::ref_box_renderer(ctx, -1);
}
void simple_object::draw(cgv::render::context& ctx)
{
	poseable::draw(ctx);
	// show box
	auto& br = cgv::render::ref_box_renderer(ctx);
	br.set_render_style(brs);
	if (brs.rounding)
		br.set_prog(prog);
	br.set_position(ctx, position);
	br.set_color_array(ctx, &color, 1);
	br.set_secondary_color(ctx, get_modified_color(color));
	br.set_extent(ctx, extent);
	br.set_rotation_array(ctx, &rotation, 1);
	br.render(ctx, 0, 1);
}

void simple_object::on_set(void* member_ptr)
{
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

void simple_object::create_gui()
{
	add_decorator(get_name(), "heading", "level=2");
	add_member_control(this, "color", color);
	add_member_control(this, "width", extent[0], "value_slider", "min=0.01;max=1;log=true");
	add_member_control(this, "height", extent[1], "value_slider", "min=0.01;max=1;log=true");
	add_member_control(this, "depth", extent[2], "value_slider", "min=0.01;max=1;log=true");
	add_gui("rotation", rotation, "direction", "options='min=-1;max=1;ticks=true'");
	add_member_control(this, "Active Gizmo", active_gizmo_ui, "dropdown", "enums='None,Translation,Scaling,Rotation'");
	if (begin_tree_node("style", brs)) {
		align("\a");
		add_gui("brs", brs);
		align("\b");
		end_tree_node(brs);
	}
	poseable::create_gui();
}
