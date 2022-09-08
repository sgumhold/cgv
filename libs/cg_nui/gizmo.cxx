#include "gizmo.h"
using namespace cgv::render;

mat4 cgv::nui::gizmo::compute_draw_correction_transformation(vec3& scale)
{
	vec3 obj_translation;
	quat obj_rotation;
	vec3 obj_scale;
	transforming::extract_transform_components(transforming::get_global_model_transform(anchor_obj), obj_translation, obj_rotation, obj_scale);
	quat obj_inverse_rotation = obj_rotation.inverse();
	vec3 obj_inverse_scale = vec3(1.0f / obj_scale.x(), 1.0f / obj_scale.y(), 1.0f / obj_scale.z());

	scale = obj_scale;

	mat4 final_rotation;
	final_rotation.identity();
	if (use_absolute_rotation)
		final_rotation = obj_inverse_rotation.get_homogeneous_matrix();
	if (anchor_rotation_ptr)
		final_rotation = anchor_rotation_ptr->get_homogeneous_matrix() * final_rotation;
	else if (anchor_rotation_ptr_ptr)
		final_rotation = (*anchor_rotation_ptr_ptr)->get_homogeneous_matrix() * final_rotation;

	mat4 final_translation;
	final_translation.identity();
	if (anchor_position_ptr)
		final_translation = cgv::math::translate4(*anchor_position_ptr);
	else if (anchor_rotation_ptr_ptr)
		final_translation = cgv::math::translate4(**anchor_position_ptr_ptr);

	return final_translation * final_rotation * cgv::math::scale4(obj_inverse_scale);
}

mat4 cgv::nui::gizmo::compute_interaction_correction_transformation(vec3& scale)
{
	vec3 obj_translation;
	quat obj_rotation;
	vec3 obj_scale;
	transforming::extract_transform_components(transforming::get_global_model_transform(this), obj_translation, obj_rotation, obj_scale);

	scale = obj_scale;

	mat4 final_inverse_rotation;
	final_inverse_rotation.identity();
	if (use_absolute_rotation)
		final_inverse_rotation = obj_rotation.get_homogeneous_matrix();

	return final_inverse_rotation * cgv::math::scale4(obj_scale);
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
	// TODO: Find out how to get draw called without a group parent
	cgv::base::group_ptr grp = anchor_obj->cast<group>();
	if (grp)
		grp->append_child(this);
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

void cgv::nui::gizmo::set_use_absolute_rotation(bool value)
{
	use_absolute_rotation = value;
}

void cgv::nui::gizmo::set_is_anchor_influenced_by_gizmo(bool value)
{
	is_anchor_influenced_by_gizmo = value;
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

	
	vec3 scale;
	mat4 correction_transform = compute_interaction_correction_transformation(scale);

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Reverse scale in the attached object's coordinate system so that the gizmo's handles aren't scaled
	return _compute_closest_point(correction_transform * vec4(point, 1), prj_point, prj_normal, primitive_idx, scale, view_matrix);
}

bool cgv::nui::gizmo::compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx)
{
	if (!is_attached)
		return false;

	vec3 scale;
	mat4 correction_transform = compute_interaction_correction_transformation(scale);

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Reverse scale in the attached object's coordinate system so that the gizmo's handles aren't scaled
	bool result = _compute_intersection(correction_transform * vec4(ray_start, 1), correction_transform * vec4(ray_direction, 0), hit_param,
		hit_normal, primitive_idx, scale, view_matrix);
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

	vec3 scale;
	mat4 correction_transform = compute_draw_correction_transformation(scale);

	mat4 view_matrix;
	view_matrix.identity(); // TODO: Get actual camera transform

	// Reverse scale (and possibly rotation) in the attached object's coordinate system so that the gizmo's handles aren't scaled (or rotated)
	ctx.push_modelview_matrix();
	ctx.mul_modelview_matrix(correction_transform);
	_draw(ctx, scale, view_matrix);
	ctx.pop_modelview_matrix();
}

const mat4& cgv::nui::gizmo::get_model_transform() const
{
	mat4& transform = *(new mat4());
	transform.identity();
	if (anchor_rotation_ptr)
		transform = anchor_rotation_ptr->get_homogeneous_matrix() * transform;
	else if (anchor_rotation_ptr_ptr)
		transform = (*anchor_rotation_ptr_ptr)->get_homogeneous_matrix() * transform;
	if (anchor_position_ptr)
		transform = cgv::math::translate4(*anchor_position_ptr) * transform;
	else if (anchor_position_ptr_ptr)
		transform = cgv::math::translate4(**anchor_position_ptr_ptr) * transform;
	return transform;
}

const mat4& cgv::nui::gizmo::get_inverse_model_transform() const
{
	mat4& transform = *(new mat4());
	transform.identity();
	if (anchor_rotation_ptr)
		transform = anchor_rotation_ptr->inverse().get_homogeneous_matrix() * transform;
	else if (anchor_rotation_ptr_ptr)
		transform = (*anchor_rotation_ptr_ptr)->inverse().get_homogeneous_matrix() * transform;
	if (anchor_position_ptr)
		transform = cgv::math::translate4(-1.0f * (*anchor_position_ptr)) * transform;
	else if (anchor_position_ptr_ptr)
		transform = cgv::math::translate4(-1.0f * (**anchor_position_ptr_ptr)) * transform;
	return transform;
}

vec3 cgv::nui::gizmo::get_local_position() const
{
	if (anchor_position_ptr)
		return *anchor_position_ptr;
	else if (anchor_position_ptr_ptr)
		return **anchor_position_ptr_ptr;
	return vec3(0.0f);
}

quat cgv::nui::gizmo::get_local_rotation() const
{
	if (anchor_rotation_ptr)
		return *anchor_rotation_ptr;
	else if (anchor_rotation_ptr_ptr)
		return **anchor_rotation_ptr_ptr;
	return quat();
}
vec3 cgv::nui::gizmo::get_local_scale() const
{
	return vec3(1.0f);
}
