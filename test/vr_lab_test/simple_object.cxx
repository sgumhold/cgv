#include "simple_object.h"
#include <cgv/math/proximity.h>
#include <cgv/math/intersection.h>

#include <cg_nui/debug_visualization_helper.h>

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
	cgv::nui::poseable(_name), transforming(), translatable(), rotatable(&rotation), scalable(&extent), rotation(_rotation), extent(_extent), color(_color)
{
	name = _name;

	positions.push_back(_position);
	set_position(_position);

	//debug_point = position + 0.5f*extent;
	brs.rounding = true;
	brs.default_radius = 0.02f;

	active_gizmo_ui = active_gizmo;
}

std::string simple_object::get_type_name() const
{
	return "simple_object";
}

void simple_object::initialize_gizmos(cgv::base::node_ptr root, cgv::base::node_ptr anchor, bool anchor_influenced_by_gizmo, bool root_influenced_by_gizmo)
{
	trans_gizmo = new cgv::nui::translation_gizmo();
	// As the gizmo is created after the hierarchy was traversed the context has to be set manually
	trans_gizmo->set_context(this->get_context());
	trans_gizmo->set_anchor_object(anchor);
	trans_gizmo->set_root_object(root);
	//trans_gizmo->set_anchor_offset_position(vec3(0.0f, 0.0f, 0.0f));
	//trans_gizmo->set_root_offset_position(vec3(0.0f, 0.0f, 0.0f));
	quat rot;
	vec3 n = vec3(1.0f, 0.0f, 1.0f);
	n.normalize();
	rot.set_normal(n);
	rot.normalize();
	//trans_gizmo->set_anchor_offset_rotation(rot);
	n = vec3(1.0f, 0.0f, 1.0f);
	n.normalize();
	rot.set_normal(n);
	rot.normalize();
	//trans_gizmo->set_root_offset_rotation(rot);
	trans_gizmo->set_position_reference(this);
	trans_gizmo->set_value_object(this);
	trans_gizmo->set_is_anchor_influenced_by_gizmo(anchor_influenced_by_gizmo);
	trans_gizmo->set_is_root_influenced_by_gizmo(root_influenced_by_gizmo);
	trans_gizmo->set_use_root_for_rotation(false);
	trans_gizmo->set_use_root_for_position(false);
	trans_gizmo->set_axes_directions({ vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) });
	trans_gizmo->set_axes_positions(
		{ vec3(0.5f, 0.0f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 0.0f, 0.5f) },
		{ vec3(0.02f, 0.0f, 0.0f), vec3(0.0f, 0.02f, 0.0f), vec3(0.0f, 0.0f, 0.02f) }
	);
	if (active_gizmo == ActiveGizmoOptions::AGO_TRANSLATION)
		trans_gizmo->attach();

	rot_gizmo = new cgv::nui::rotation_gizmo();
	// As the gizmo is created after the hierarchy was traversed the context has to be set manually
	rot_gizmo->set_context(this->get_context());
	rot_gizmo->set_anchor_object(anchor);
	rot_gizmo->set_root_object(root);
	rot_gizmo->set_rotation_reference(this);
	rot_gizmo->set_is_anchor_influenced_by_gizmo(anchor_influenced_by_gizmo);
	rot_gizmo->set_is_root_influenced_by_gizmo(root_influenced_by_gizmo);
	trans_gizmo->set_use_root_for_rotation(false);
	trans_gizmo->set_use_root_for_position(false);
	rot_gizmo->set_axes_directions({ vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) });
	if (active_gizmo == ActiveGizmoOptions::AGO_ROTATION)
		rot_gizmo->attach();

	scale_gizmo = new cgv::nui::scaling_gizmo();
	// As the gizmo is created after the hierarchy was traversed the context has to be set manually
	scale_gizmo->set_context(this->get_context());
	scale_gizmo->set_anchor_object(anchor);
	scale_gizmo->set_root_object(root);
	scale_gizmo->set_scale_reference(this);
	scale_gizmo->set_is_anchor_influenced_by_gizmo(anchor_influenced_by_gizmo);
	scale_gizmo->set_is_root_influenced_by_gizmo(root_influenced_by_gizmo);
	trans_gizmo->set_use_root_for_rotation(false);
	trans_gizmo->set_use_root_for_position(false);
	scale_gizmo->set_axes_directions({ vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0) });
	scale_gizmo->set_axes_positions(
		{ vec3(0.5f, 0.0f, 0.0f), vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 0.0f, 0.5f) },
		{ vec3(0.02f, 0.0f, 0.0f), vec3(0.0f, 0.02f, 0.0f), vec3(0.0f, 0.0f, 0.02f) }
	);
	scale_gizmo->configure_scale_limits(vec3(0.05f), vec3(2.0f));
	if (active_gizmo == ActiveGizmoOptions::AGO_SCALING)
		scale_gizmo->attach();
}

bool simple_object::_compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx)
{
	for (int i = 0; i < 3; ++i)
		//prj_point[i] = std::max(-0.5f * extent[i], std::min(0.5f * extent[i], prj_point[i]));
		prj_point[i] = std::max(-0.5f, std::min(0.5f, prj_point[i]));
	return true;
}

bool simple_object::_compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	//auto result = cgv::math::ray_box_intersection(ray_start, ray_direction, -0.5f * extent, 0.5f * extent);
	auto result = cgv::math::ray_box_intersection(ray_start, ray_direction, vec3(-0.5f), vec3(0.5f));
	if (result.hit) {
		hit_param = result.t_near;
		return true;
	}
	return false;
}

bool simple_object::init(cgv::render::context& ctx)
{
	poseable::init(ctx);

	if (get_name() == "blue") {
		auto& dvh = cgv::nui::ref_debug_visualization_helper(ctx, 1);
		debug_coord_system_handle0 = dvh.register_debug_value_coordinate_system();
		auto config = dvh.get_config_debug_value_coordinate_system(debug_coord_system_handle0);
		config.show_translation = false;
		config.position = vec3(0.4f, 2.0f, 0.0f);
		dvh.set_config_debug_value_coordinate_system(debug_coord_system_handle0, config);
	}

	auto& br = cgv::render::ref_box_renderer(ctx, 1);
	if (prog.is_linked())
		return true;
	return br.build_program(ctx, prog, brs);
}
void simple_object::clear(cgv::render::context& ctx)
{
	auto& dvh = cgv::nui::ref_debug_visualization_helper();
	if (get_name() == "blue") {
		dvh.deregister_debug_value(debug_coord_system_handle0);
	}
	cgv::nui::ref_debug_visualization_helper(ctx, -1);
	cgv::render::ref_box_renderer(ctx, -1);
}
void simple_object::draw(cgv::render::context& ctx)
{
	if (get_name() == "blue") {
		cgv::nui::ref_debug_visualization_helper().update_debug_value_coordinate_system(debug_coord_system_handle0, get_global_model_transform(this));
	}

	ctx.push_modelview_matrix();
	ctx.mul_modelview_matrix(get_model_transform());

	poseable::draw(ctx);
	// show box
	auto& br = cgv::render::ref_box_renderer(ctx);
	br.set_render_style(brs);
	if (brs.rounding)
		br.set_prog(prog);
	br.set_position(ctx, vec3(0.0f));
	br.set_color_array(ctx, &color, 1);
	br.set_secondary_color(ctx, get_modified_color(color));
	//br.set_extent(ctx, extent);
	br.set_extent(ctx, vec3(1.0));
	br.render(ctx, 0, 1);
}

void simple_object::finish_draw(cgv::render::context& context)
{
	context.pop_modelview_matrix();
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
			case ActiveGizmoOptions::AGO_ROTATION: rot_gizmo->attach(); break;
			case ActiveGizmoOptions::AGO_SCALING: scale_gizmo->attach(); break;
			}
			active_gizmo = active_gizmo_ui;
		}
	}
	if (member_ptr == &extent) {
		if (extent.y() < 0.1) {
			extent[1] = 0.1;
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

cgv::render::render_types::mat4 simple_object::get_model_transform() const
{
	//return transforming::construct_transform_from_components(get_position(), get_rotation(), vec3(1.0f));
	return transforming::construct_transform_from_components(get_position(), get_rotation(), get_scale());
}

cgv::render::render_types::mat4 simple_object::get_inverse_model_transform() const
{
	//const mat4& transform = transforming::construct_inverse_transform_from_components(-1.0f * get_position(), get_rotation().inverse(), vec3(1.0));
	return transforming::construct_inverse_transform_from_components(-1.0f * get_position(), get_rotation().inverse(), vec3(1.0) / get_scale());
}
