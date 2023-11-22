#include "debug_visualization_helper.h"
#include "gizmo.h"
using namespace cgv::render;

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
	if (!value_obj) {
		std::cout << "Invalid Configuration: A gizmo has to have a valid value object" << std::endl;
		return false;
	}
	return true;
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

	// Attach gizmo to anchor obj
	node::set_parent(anchor_obj);
	cgv::base::group_ptr grp = anchor_obj->cast<group>();
	if (grp)
		grp->append_child(this);
}

void cgv::nui::gizmo::set_root_object(cgv::base::node_ptr _root_obj)
{
	root_obj = _root_obj;
}

void cgv::nui::gizmo::set_value_object(cgv::base::node_ptr _value_obj)
{
	value_obj = _value_obj;
}

void cgv::nui::gizmo::set_anchor_offset_position(vec3 _offset_position)
{
	anchor_offset_position = _offset_position;
	anchor_offset_position_ptr = &anchor_offset_position;
}

void cgv::nui::gizmo::set_anchor_offset_rotation(quat _offset_rotation)
{
	anchor_offset_rotation = _offset_rotation;
	anchor_offset_rotation_ptr = &anchor_offset_rotation;
}

void cgv::nui::gizmo::set_anchor_offset_position(const vec3* _offset_position_ptr)
{
	anchor_offset_position_ptr = _offset_position_ptr;
}

void cgv::nui::gizmo::set_anchor_offset_rotation(const quat* _offset_rotation_ptr)
{
	anchor_offset_rotation_ptr = _offset_rotation_ptr;
}

void cgv::nui::gizmo::set_anchor_offset_position(const vec3** _offset_position_ptr_ptr)
{
	anchor_offset_position_ptr_ptr = _offset_position_ptr_ptr;
}

void cgv::nui::gizmo::set_anchor_offset_rotation(const quat** _offset_rotation_ptr_ptr)
{
	anchor_offset_rotation_ptr_ptr = _offset_rotation_ptr_ptr;
}

void cgv::nui::gizmo::set_root_offset_position(vec3 _offset_position)
{
	root_offset_position = _offset_position;
	root_offset_position_ptr = &root_offset_position;
}

void cgv::nui::gizmo::set_root_offset_rotation(quat _offset_rotation)
{
	root_offset_rotation = _offset_rotation;
	root_offset_rotation_ptr = &root_offset_rotation;
}

void cgv::nui::gizmo::set_root_offset_position(const vec3* _offset_position_ptr)
{
	root_offset_position_ptr = _offset_position_ptr;
}

void cgv::nui::gizmo::set_root_offset_rotation(const quat* _offset_rotation_ptr)
{
	root_offset_rotation_ptr = _offset_rotation_ptr;
}

void cgv::nui::gizmo::set_root_offset_position(const vec3** _offset_position_ptr_ptr)
{
	root_offset_position_ptr_ptr = _offset_position_ptr_ptr;
}

void cgv::nui::gizmo::set_root_offset_rotation(const quat** _offset_rotation_ptr_ptr)
{
	root_offset_rotation_ptr_ptr = _offset_rotation_ptr_ptr;
}

void cgv::nui::gizmo::set_is_anchor_influenced_by_gizmo(bool value)
{
	is_anchor_influenced_by_gizmo = value;
}

void cgv::nui::gizmo::set_is_root_influenced_by_gizmo(bool value)
{
	is_root_influenced_by_gizmo = value;
}

void cgv::nui::gizmo::set_use_root_for_rotation(bool value)
{
	use_root_for_rotation = value;
}

void cgv::nui::gizmo::set_use_root_for_position(bool value)
{
	use_root_for_position = value;
}

//void cgv::nui::gizmo::set_use_root_for_offset_rotation(bool value)
//{
//	use_root_for_offset_rotation = value;
//}
//
//void cgv::nui::gizmo::set_use_root_for_offset_position(bool value)
//{
//	use_root_for_offset_position = value;
//}

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

	vec3 anchor_scale = anchor_scale_component / root_scale_component;

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Correction is already applied by dispatcher. Anchor scale has to be provided.
	return _compute_closest_point(point, prj_point, prj_normal, primitive_idx,
		anchor_scale, view_matrix);
}

bool cgv::nui::gizmo::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	if (!is_attached)
		return false;

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

	vec3 anchor_scale = anchor_scale_component / root_scale_component;

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Correction is already applied by dispatcher. Anchor scale has to be provided.
	bool result = _compute_intersection(ray_start, ray_direction, hit_param,
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

	
	vec3 anchor_scale = anchor_scale_component / root_scale_component;

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Apply the correction to the gizmo transform
	ctx.push_modelview_matrix();
	ctx.mul_modelview_matrix(get_model_transform());

	_draw(ctx, anchor_scale, view_matrix);
	
}

void cgv::nui::gizmo::finish_draw(context& ctx)
{
	if (!is_attached)
		return;

	ctx.pop_modelview_matrix();
}

mat4 cgv::nui::gizmo::get_model_transform() const
{
	vec3 anchor_obj_global_translation;
	quat anchor_obj_global_rotation;
	vec3 anchor_obj_global_scale;
	transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj),
		anchor_obj_global_translation, anchor_obj_global_rotation, anchor_obj_global_scale);

	vec3 anchor_obj_parent_global_translation;
	quat anchor_obj_parent_global_rotation;
	vec3 anchor_obj_parent_global_scale;
	transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj->get_parent()),
		anchor_obj_parent_global_translation, anchor_obj_parent_global_rotation, anchor_obj_parent_global_scale);

	vec3 root_obj_global_translation;
	quat root_obj_global_rotation;
	vec3 root_obj_global_scale;
	transforming::extract_transform_components(transforming::get_global_model_transform(root_obj),
		root_obj_global_translation, root_obj_global_rotation, root_obj_global_scale);

	vec3 anchor_root_diff_translation = root_obj_global_translation - anchor_obj_global_translation;
	quat anchor_root_diff_rotation = anchor_obj_global_rotation.inverse() * root_obj_global_rotation;
	vec3 anchor_root_diff_scale = root_obj_global_scale / anchor_obj_global_scale;
	vec3 anchor_obj_local_translation = anchor_obj_global_translation - anchor_obj_parent_global_translation;
	quat anchor_obj_local_rotation = anchor_obj_parent_global_rotation.inverse() * anchor_obj_global_rotation;
	vec3 anchor_obj_local_scale = anchor_obj_global_scale / anchor_obj_parent_global_scale;

	mat4 transform;
	transform.identity();

	// Apply position difference between anchor and root
	if (use_root_for_position) {
		// This fixes the rotation issues but not the scale-dependency of the position
		//transform = cgv::math::translate4(anchor_obj_global_rotation.inverse().get_rotated(anchor_root_diff_translation)) * transform;
		// This fixes the position initially and for table scales but not for blue box scale
		transform = cgv::math::translate4(anchor_obj_global_rotation.inverse().get_rotated(anchor_root_diff_translation) / anchor_obj_parent_global_scale) * transform;
	}

	// Apply rotation difference between anchor and root
	if (use_root_for_rotation) {
		transform = anchor_root_diff_rotation.get_homogeneous_matrix() * transform;
	}
	// Apply scale difference between anchor and root
	transform = cgv::math::scale4(anchor_root_diff_scale) * transform;
	// The above works but isn't this actually correct?
	//transform = anchor_obj_global_rotation.get_homogeneous_matrix() * cgv::math::scale4(anchor_root_diff_scale) * anchor_obj_global_rotation.inverse().get_homogeneous_matrix() * transform;

	// TODO: Not yet tested
	// Apply anchor offset rotation
	const quat* offset_rotation_ptr = nullptr;
	if (anchor_offset_rotation_ptr)
		offset_rotation_ptr = anchor_offset_rotation_ptr;
	else if (anchor_offset_rotation_ptr_ptr)
		offset_rotation_ptr = *anchor_offset_rotation_ptr_ptr;
	if (offset_rotation_ptr) {
		if (use_root_for_rotation) {
			// Rotate anchor offset rotation by anchor-root-diff to get into anchor coordinate system
			transform = (anchor_root_diff_rotation * (*offset_rotation_ptr)).get_homogeneous_matrix() * transform;
		}
		else {
			transform = offset_rotation_ptr->get_homogeneous_matrix() * transform;
		}
	}
	// TODO: Not yet tested
	// Apply root offset rotation
	offset_rotation_ptr = nullptr;
	if (root_offset_rotation_ptr)
		offset_rotation_ptr = root_offset_rotation_ptr;
	else if (root_offset_rotation_ptr_ptr)
		offset_rotation_ptr = *root_offset_rotation_ptr_ptr;
	if (offset_rotation_ptr) {
		if (use_root_for_rotation) {
			transform = offset_rotation_ptr->get_homogeneous_matrix() * transform;
		}
		else {
			// Rotate root offset rotation by inverse anchor-root-diff to get into root coordinate system
			transform = (anchor_root_diff_rotation.inverse() * (*offset_rotation_ptr)).get_homogeneous_matrix() * transform;
		}
	}
	// TODO: Not yet tested
	// Apply anchor offset position
	const vec3* offset_position_ptr = nullptr;
	if (anchor_offset_position_ptr)
		offset_position_ptr = anchor_offset_position_ptr;
	else if (anchor_offset_position_ptr_ptr)
		offset_position_ptr = *anchor_offset_position_ptr_ptr;
	if (offset_position_ptr) {
		if (use_root_for_position) {
			// Rotate anchor offset position by anchor-root-diff to get into anchor coordinate system
			transform = cgv::math::translate4(anchor_root_diff_rotation.get_rotated(*offset_position_ptr)) * transform;
		}
		else {
			transform = cgv::math::translate4(*offset_position_ptr) * transform;
		}
	}
	// TODO: Not yet tested
	// Apply root offset position
	offset_position_ptr = nullptr;
	if (root_offset_position_ptr)
		offset_position_ptr = root_offset_position_ptr;
	else if (root_offset_position_ptr_ptr)
		offset_position_ptr = *root_offset_position_ptr_ptr;
	if (offset_position_ptr) {
		if (use_root_for_position) {
			transform = cgv::math::translate4(*offset_position_ptr) * transform;
		}
		else {
			// Rotate root offset position by inverse anchor-root-diff to get into root coordinate system
			transform = cgv::math::translate4(anchor_root_diff_rotation.inverse().get_rotated(*offset_position_ptr)) * transform;
		}
	}

	return transform;
}

mat4 cgv::nui::gizmo::gizmo_to_other_object_transform(cgv::base::node_ptr other_object)
{
	return get_global_inverse_model_transform(value_obj->get_parent())
		* anchor_obj->get_parent()->get_interface<transforming>()->get_model_transform()
		* anchor_obj->get_interface<transforming>()->get_model_transform()
		* get_model_transform();
}

vec3 cgv::nui::gizmo::gizmo_to_value_transform_point(const vec3& point)
{
	return gizmo_to_other_object_transform(value_obj) * vec4(point, 1.0f);
}

vec3 cgv::nui::gizmo::gizmo_to_value_transform_vector(const vec3& vector)
{
	return gizmo_to_other_object_transform(value_obj) * vec4(vector, 0.0f);
}

vec3 cgv::nui::gizmo::gizmo_to_value_parent_transform_point(const vec3& point)
{
	cgv::base::node_ptr parent = value_obj->get_parent();
	if (parent)
		return gizmo_to_other_object_transform(parent) * vec4(point, 1.0f);
	return gizmo_to_other_object_transform(value_obj) * vec4(point, 1.0f);
}

vec3 cgv::nui::gizmo::gizmo_to_value_parent_transform_vector(const vec3& vector)
{
	cgv::base::node_ptr parent = value_obj->get_parent();
	if (parent)
		return gizmo_to_other_object_transform(parent) * vec4(vector, 0.0f);
	return gizmo_to_other_object_transform(value_obj) * vec4(vector, 0.0f);
}
