#include "gizmo.h"
using namespace cgv::render;


void cgv::nui::gizmo::on_ii_during_focus_changed(hid_identifier changed_key)
{
	vec3 scale;
	mat4 correction_transform = compute_interaction_correction_transformation(scale);
	ii_at_grab.hid_position = correction_transform * vec4(interactable::ii_at_grab.hid_position, 1.0);
	ii_at_grab.hid_direction = correction_transform * vec4(interactable::ii_at_grab.hid_direction, 0.0);
	ii_at_grab.query_point = correction_transform * vec4(interactable::ii_at_grab.query_point, 1.0);
	ii_during_focus[changed_key].hid_position = correction_transform * vec4(interactable::ii_during_focus[changed_key].hid_position, 1.0);
	ii_during_focus[changed_key].hid_direction = correction_transform * vec4(interactable::ii_during_focus[changed_key].hid_direction, 0.0);
	ii_during_focus[changed_key].query_point = correction_transform * vec4(interactable::ii_during_focus[changed_key].query_point, 1.0);
}

void cgv::nui::gizmo::on_ii_at_grab_changed()
{
	vec3 scale;
	mat4 correction_transform = compute_interaction_correction_transformation(scale);
	ii_at_grab.hid_position = correction_transform * vec4(interactable::ii_at_grab.hid_position, 1.0);
	ii_at_grab.hid_direction = correction_transform * vec4(interactable::ii_at_grab.hid_direction, 0.0);
	ii_at_grab.query_point = correction_transform * vec4(interactable::ii_at_grab.query_point, 1.0);
}

bool cgv::nui::gizmo::validate_configuration()
{
	if (!anchor_obj) {
		std::cout << "Invalid Configuration: A gizmo has to have a valid anchor object" << std::endl;
		return false;
	}
	if (!root_obj) {
		std::cout << "Invalid Configuration: A gizmo has to have a valid root object" << std::endl;
		return false;
	}
	return true;
}

mat4 cgv::nui::gizmo::compute_draw_correction_transformation(vec3& scale)
{
	vec3 root_translation_component;
	quat root_rotation_component;
	vec3 root_scale_component;
	transforming::extract_transform_components(transforming::get_global_model_transform(root_obj),
		root_translation_component, root_rotation_component, root_scale_component);

	vec3 anchor_translation_component;
	quat anchor_rotation_component;
	vec3 anchor_scale_component;
	transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj),
		anchor_translation_component, anchor_rotation_component, anchor_scale_component);

	mat4 scale_correction;
	//if (!use_root_position) {
		scale = anchor_scale_component / root_scale_component;
		scale_correction = cgv::math::scale4(root_scale_component / anchor_scale_component);
	//}
	//else {
	//	scale_correction.identity();
	//	scale = vec3(1.0f);
	//}

	mat4 rotation_correction;
	if (use_root_rotation) {
		rotation_correction = (root_rotation_component.inverse() * anchor_rotation_component).inverse().get_homogeneous_matrix();
	}
	else {
		rotation_correction.identity();
	}

	mat4 position_correction;
	if (use_root_position) {
		position_correction = cgv::math::translate4(root_translation_component - anchor_translation_component);
	}
	else {
		position_correction.identity();
	}
	//if (anchor_position_ptr)
	//	position_correction = cgv::math::translate4(*anchor_position_ptr) * position_correction;
	//else if (anchor_position_ptr_ptr)
	//	position_correction = cgv::math::translate4(**anchor_position_ptr_ptr) * position_correction;
	
	return rotation_correction * scale_correction * position_correction;
}

mat4 cgv::nui::gizmo::compute_interaction_correction_transformation(vec3& scale)
{
	vec3 root_translation_component;
	quat root_rotation_component;
	vec3 root_scale_component;
	transforming::extract_transform_components(transforming::get_global_model_transform(root_obj),
		root_translation_component, root_rotation_component, root_scale_component);

	vec3 anchor_translation_component;
	quat anchor_rotation_component;
	vec3 anchor_scale_component;
	transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj),
		anchor_translation_component, anchor_rotation_component, anchor_scale_component);

	mat4 scale_correction;
	//if (!use_root_position) {
		scale = anchor_scale_component;
		scale_correction = cgv::math::scale4(anchor_scale_component / root_scale_component);
	//}
	//else {
	//	scale_correction.identity();
	//	scale = vec3(1.0f);
	//}
	
	mat4 rotation_correction;
	if (use_root_rotation) {
		rotation_correction = (anchor_rotation_component.inverse() * root_rotation_component).inverse().get_homogeneous_matrix();
	}
	else {
		rotation_correction.identity();
	}

	mat4 position_correction;
	if (use_root_position) {
		position_correction = cgv::math::translate4(anchor_translation_component - root_translation_component);
	}
	else {
		position_correction.identity();
	}
	//if (anchor_position_ptr)
	//	position_correction = cgv::math::translate4(-*anchor_position_ptr) * position_correction;
	//else if (anchor_position_ptr_ptr)
	//	position_correction = cgv::math::translate4(-**anchor_position_ptr_ptr) * position_correction;

	return rotation_correction * scale_correction * position_correction;
}

void cgv::nui::gizmo::attach()
{
	if (!validate_configuration())
	{
		std::cout << "Gizmo could not be attached due to invalid configuration" << std::endl;
		return;
	}
	is_attached = true;

	precompute_geometry();
}

void cgv::nui::gizmo::detach()
{
	if (!is_attached)
		return;
	is_attached = false;
}

void cgv::nui::gizmo::set_anchor_object(cgv::base::node_ptr _anchor_obj)
{
	anchor_obj = _anchor_obj;
	node::set_parent(anchor_obj);
	cgv::base::group_ptr grp = anchor_obj->cast<group>();
	if (grp)
		grp->append_child(this);
}

void cgv::nui::gizmo::set_root_object(cgv::base::node_ptr _root_obj)
{
	root_obj = _root_obj;
}

void cgv::nui::gizmo::set_anchor_offset_position(vec3 _anchor_position)
{
	anchor_position = _anchor_position;
	anchor_position_ptr = &anchor_position;
}

void cgv::nui::gizmo::set_anchor_offset_rotation(quat _anchor_rotation)
{
	anchor_rotation = _anchor_rotation;
	anchor_rotation_ptr = &anchor_rotation;
}

void cgv::nui::gizmo::set_anchor_offset_position(const vec3* _anchor_position_ptr)
{
	anchor_position_ptr = _anchor_position_ptr;
}

void cgv::nui::gizmo::set_anchor_offset_rotation(const quat* _anchor_rotation_ptr)
{
	anchor_rotation_ptr = _anchor_rotation_ptr;
}

void cgv::nui::gizmo::set_anchor_offset_position(const vec3** _anchor_position_ptr_ptr)
{
	anchor_position_ptr_ptr = _anchor_position_ptr_ptr;
}

void cgv::nui::gizmo::set_anchor_offset_rotation(const quat** _anchor_rotation_ptr_ptr)
{
	anchor_rotation_ptr_ptr = _anchor_rotation_ptr_ptr;
}

void cgv::nui::gizmo::set_is_anchor_influenced_by_gizmo(bool value)
{
	is_anchor_influenced_by_gizmo = value;
}

void cgv::nui::gizmo::set_is_root_influenced_by_gizmo(bool value)
{
	is_root_influenced_by_gizmo = value;
}

void cgv::nui::gizmo::set_use_root_rotation(bool value)
{
	use_root_rotation = value;
}

void cgv::nui::gizmo::set_use_root_position(bool value)
{
	use_root_position = value;
}

void cgv::nui::gizmo::set_on_set_object(cgv::base::base_ptr _on_set_obj)
{
	on_set_obj = _on_set_obj;
}

bool cgv::nui::gizmo::focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info)
{
	if (!is_attached)
		return false;
	return interactable::focus_change(action, rfa, demand, e, dis_info);
}

bool cgv::nui::gizmo::handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request)
{
	if (!is_attached)
		return false;

	return interactable::handle(e, dis_info, request);
}

bool cgv::nui::gizmo::compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx)
{
	if (!is_attached)
		return false;

	
	vec3 anchor_scale;
	vec3 root_scale_correction;
	const mat4 correction_transform = compute_interaction_correction_transformation(anchor_scale);

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Apply correction transform to match configuration
	return _compute_closest_point(correction_transform * vec4(point, 1), prj_point, prj_normal, primitive_idx,
	                              anchor_scale, view_matrix);
}

bool cgv::nui::gizmo::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	if (!is_attached)
		return false;

	vec3 anchor_scale;
	vec3 root_scale_correction;
	const mat4 correction_transform = compute_interaction_correction_transformation(anchor_scale);

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Apply correction transform to match configuration
	bool result = _compute_intersection(correction_transform * vec4(ray_start, 1), correction_transform * vec4(ray_direction, 0), hit_param,
	                                    hit_normal, primitive_idx, anchor_scale, view_matrix);
	// Make sure there is still an intersection if a handle is grabbed but the ray temporarily doesn't actually intersect it.
	if (!result && state == state_enum::grabbed || state == state_enum::triggered) {
		vec3 v = ii_during_focus[activating_hid_id].query_point - ray_start;
		vec3 n = ray_direction;
		hit_param = (math::dot(v, n) / math::dot(n, n) * n).length();
		primitive_idx = prim_idx;
		return true;
	}
	return result;
}

void cgv::nui::gizmo::draw(cgv::render::context& ctx)
{
	if (!is_attached)
		return;

	vec3 anchor_scale;
	vec3 root_scale_correction;
	const mat4 correction_transform = compute_draw_correction_transformation(anchor_scale);

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Apply correction transformation to match the configuration
	ctx.push_modelview_matrix();
	ctx.mul_modelview_matrix(get_model_transform());

	ctx.mul_modelview_matrix(correction_transform);

	_draw(ctx, anchor_scale, view_matrix);
	ctx.pop_modelview_matrix();
}

mat4 cgv::nui::gizmo::get_model_transform() const
{
	mat4 transform;
	transform.identity();
	//if (anchor_rotation_ptr)
	//	transform = anchor_rotation_ptr->get_homogeneous_matrix() * transform;
	//else if (anchor_rotation_ptr_ptr)
	//	transform = (*anchor_rotation_ptr_ptr)->get_homogeneous_matrix() * transform;
	//if (anchor_position_ptr)
	//	transform = cgv::math::translate4(*anchor_position_ptr) * transform;
	//else if (anchor_position_ptr_ptr)
	//	transform = cgv::math::translate4(**anchor_position_ptr_ptr) * transform;
	return transform;
}

mat4 cgv::nui::gizmo::get_inverse_model_transform() const
{
	mat4 transform;
	transform.identity();
	//if (anchor_rotation_ptr)
	//	transform = anchor_rotation_ptr->inverse().get_homogeneous_matrix() * transform;
	//else if (anchor_rotation_ptr_ptr)
	//	transform = (*anchor_rotation_ptr_ptr)->inverse().get_homogeneous_matrix() * transform;
	//if (anchor_position_ptr)
	//	transform = cgv::math::translate4(-1.0f * (*anchor_position_ptr)) * transform;
	//else if (anchor_position_ptr_ptr)
	//	transform = cgv::math::translate4(-1.0f * (**anchor_position_ptr_ptr)) * transform;
	return transform;
}

vec3 cgv::nui::gizmo::get_local_position() const
{
	//if (anchor_position_ptr)
	//	return *anchor_position_ptr;
	//else if (anchor_position_ptr_ptr)
	//	return **anchor_position_ptr_ptr;
	return vec3(0.0f);
}

quat cgv::nui::gizmo::get_local_rotation() const
{
	//if (anchor_rotation_ptr)
	//	return *anchor_rotation_ptr;
	//else if (anchor_rotation_ptr_ptr)
	//	return **anchor_rotation_ptr_ptr;
	return quat();
}
vec3 cgv::nui::gizmo::get_local_scale() const
{
	return vec3(1.0f);
}


//bool cgv::nui::gizmo_gui_creator::create(gui::provider* p, const std::string& label, void* value_ptr,
//	const std::string& value_type, const std::string& gui_type, const std::string& options, bool* toggles)
//{
//	if (value_type != cgv::type::info::type_name<gizmo>::get_name())
//		return false;
//	if (!gui_type.empty() && gui_type != "gizmo")
//		return false;
//	cgv::base::base* base_ptr = dynamic_cast<cgv::base::base*>(p);
//	gizmo& gizmo_ref = *((gizmo*)value_ptr);
//
//	if (p->begin_tree_node("Gizmo", gizmo_ref.use_root_scale)) {
//		p->align("\a");
//		p->add_member_control(base_ptr, "use root scale", gizmo_ref.use_root_scale, "toggle");
//		p->add_member_control(base_ptr, "use root rotation", gizmo_ref.use_root_rotation, "toggle");
//		p->add_member_control(base_ptr, "use root translation", gizmo_ref.use_root_translation, "toggle");
//		p->align("\b");
//		p->end_tree_node(gizmo_ref.use_root_scale);
//	}
//
//	return true;
//}
