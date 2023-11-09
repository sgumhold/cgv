#pragma once

#include <cgv/gui/gui_creator.h>
#include <cg_nui/interactable.h>
#include <cg_nui/transforming.h>
#include <cgv/math/ftransform.h>
#include <libs/cg_nui/reusable_gizmo_functionalities.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
	class gizmo;

/// Abstract base class for gizmos.
///	A gizmo requires three references to other object in the form of node_ptr: The anchor object acts as the reference frame for the transformations
///	of the gizmo. In the scene graph the gizmo is attached to this anchor object. The root object provides an alternative reference frame that is
///	used to remove some transformations of the anchor, such as the scale.
///	Lastly, the value object provides the reference frame for the value that the gizmo changes.
///	TODO: (As this only has meaning for gizmos that change the transform of an object it might be better to move this to a subclass.)
///	Specific gizmo subclasses take additional pointers to the to-be-manipulated values.
///	A gizmo also always has an attach function that validates the entire configuration before activating the gizmo. It has to be called before
///	the gizmo can be used. There is also a detach function that deactivates the gizmo.
///
///	The main purpose of the gizmo base class is to hide all the calculations necessary between the different reference frames and to provide the
///	specific gizmo subclasses with a clean access to a "gizmo reference frame" that automatically adapts to the configuration.
class CGV_API gizmo : public cgv::nui::interactable, public cgv::nui::transforming
{
protected:
	/// Fixed offset to gizmo's position (in anchor coordinate system). Do not use directly (use through anchor_offset_position_ptr).
	vec3 anchor_offset_position;
	/// Fixed offset gizmo's rotation (in anchor coordinate system). Do not use directly (use through anchor_offset_rotation_ptr).
	quat anchor_offset_rotation;
	/// Readonly offset to gizmo's position (in anchor coordinate system)
	const vec3* anchor_offset_position_ptr{ nullptr };
	/// Readonly offset to gizmo's rotation (in anchor coordinate system)
	const quat* anchor_offset_rotation_ptr{ nullptr };
	/// Readonly offset to gizmo's position (in anchor coordinate system)
	const vec3** anchor_offset_position_ptr_ptr{ nullptr };
	/// Readonly offset to gizmo's rotation (in anchor coordinate system)
	const quat** anchor_offset_rotation_ptr_ptr{ nullptr };

	/// Fixed offset to gizmo's position (in root coordinate system). Do not use directly (use through root_offset_position_ptr).
	vec3 root_offset_position;
	/// Fixed offset gizmo's rotation (in root coordinate system). Do not use directly (use through root_offset_rotation_ptr).
	quat root_offset_rotation;
	/// Readonly offset to gizmo's position (in root coordinate system)
	const vec3* root_offset_position_ptr{ nullptr };
	/// Readonly offset to gizmo's rotation (in root coordinate system)
	const quat* root_offset_rotation_ptr{ nullptr };
	/// Readonly offset to gizmo's position (in root coordinate system)
	const vec3** root_offset_position_ptr_ptr{ nullptr };
	/// Readonly offset to gizmo's rotation (in root coordinate system)
	const quat** root_offset_rotation_ptr_ptr{ nullptr };

	/// Reference of object this gizmo is anchored to
	cgv::base::node_ptr anchor_obj{ nullptr };
	/// Reference of object that acts as the root coordinate system for this gizmo
	cgv::base::node_ptr root_obj{ nullptr };
	/// Reference of object relative to whose coordinate system the manipulated value is defined (if applicable, e.g. for positions, rotations, scales).
	cgv::base::node_ptr value_obj{ nullptr };

	/// Whether this gizmo is currently attached (all configuration validated and gizmo active)
	bool is_attached{ false };
	/// Whether the anchor object's transform is changed by the value manipulated by this gizmo
	bool is_anchor_influenced_by_gizmo{ true };
	/// Whether the root object's transform is changed by the value manipulated by this gizmo
	bool is_root_influenced_by_gizmo{ false };

protected:
	/// Whether this gizmo's orientation is based on that of the root object (as opposed to that of the anchor object).
	bool use_root_for_rotation{ false };
	/// Whether this gizmo's position is based on that of the root object (as opposed to that of the anchor object).
	bool use_root_for_position{ false };
	///// Whether the rotation offset should be applied within the coordinate system of the root object (as opposed to that of the anchor object).
	//bool use_root_for_offset_rotation{ false };
	///// Whether the position offset should be applied within the coordinate system of the root object (as opposed to that of the anchor object).
	//bool use_root_for_offset_position{ false };

protected:
	/// Reference to the object that gets notified of changing values through the on_set function
	cgv::base::base_ptr on_set_obj{ nullptr };

	// Needed to call the wrapper events on_handle_grabbed, on_handle_drag and on_handle_released
	void on_grabbed_start() override { on_handle_grabbed(); }

	void on_grabbed_drag() override { on_handle_drag(); }

	void on_grabbed_stop() override { on_handle_released(); }

	void on_triggered_start() override { on_handle_grabbed(); }

	void on_triggered_drag() override { on_handle_drag(); }

	void on_triggered_stop() override { on_handle_released(); }

	/// Event that is called when a primitive/handle of the gizmo gets grabbed by a hid.
	///	prim_idx is the primitive that was grabbed, start_position is the point it was grabbed at.
	virtual void on_handle_grabbed() {}
	/// Event that is called whenever the hid moves/rotates while grabbing the gizmo.
	///	prim_idx is the primitive that was grabbed, start_position is the point it was grabbed at,
	///	target_position is the start_position projected to reflect the movement of the hid.
	virtual void on_handle_drag() {}

	/// Event that is called when a primitive/handle of the gizmo gets released from grabbing by a hid.
	virtual void on_handle_released() {}

	/// Validate all configuration values. Return false if configuration is invalid in a way that the gizmo cannot be used or rendered.
	virtual bool validate_configuration();

	/// Compute constant values according to the configuration.
	/// Called once on attach.
	virtual void precompute_geometry() = 0;

	/// Internal draw function in local coordinate system with some transform components (according to configuration) removed that has access to the camera transform.
	///	The alternative_scale is set to unmodified anchor object scale if use_root_scale is true and to the root scale if use_root_scale is false.
	virtual void _draw(cgv::render::context& ctx, const vec3& scale, const mat4& view_matrix) {}

	/// Internal compute_closest_point function in local coordinate system with some transform components (according to configuration) removed that has access to the camera transform.
	///	The alternative_scale is set to unmodified anchor object scale if use_root_scale is true and to the root scale if use_root_scale is false.
	virtual bool _compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx, const vec3& scale,
		const mat4& view_matrix) {
		return false;
	}
	/// Internal compute_intersection function in local coordinate system with some transform components (according to configuration) removed that has access to the camera transform.
	/// The alternative_scale is set to unmodified anchor object scale if use_root_scale is true and to the root scale if use_root_scale is false.
	virtual bool _compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx,
		const vec3& scale, const mat4& view_matrix) {
		return false;
	}

	// Functions to transform from the gizmo coordinate system back to the coordinate system needed to set position, rotation, scale etc.
protected:
	/// Calculate transform from gizmo coordinate system to the coordinate system of the given object
	mat4 gizmo_to_other_object_transform(cgv::base::node_ptr other_object);
protected:
	/// Transform a point in the gizmo coordinate system to the value object coordinate system
	vec3 gizmo_to_value_transform_point(const vec3& point);
	/// Transform a vactor in the gizmo coordinate system to the value object coordinate system
	vec3 gizmo_to_value_transform_vector(const vec3& vector);
	/// Transform a point in the gizmo coordinate system to the value object's parent coordinate system
	vec3 gizmo_to_value_parent_transform_point(const vec3& point);
	/// Transform a vector in the gizmo coordinate system to the value object's parent coordinate system
	vec3 gizmo_to_value_parent_transform_vector(const vec3& vector);

public:
	gizmo(const std::string& name = "") : interactable(name) {}

	/// Validates current configuration and activates gizmo if correct.
	void attach();
	/// Deactivates gizmo
	void detach();

	// Configuration Functions

	/// Sets the object as the parent of the gizmo. The gizmo will be positioned according to any transforming interfaces in the hierarchy.
	///	This is required.
	void set_anchor_object(cgv::base::node_ptr _anchor_obj);
	/// Sets the object to act as the root coordinate system of the gizmo.
	/// It will act like a world coordinate system to allow e.g for removing the scaling of the gizmo handles.
	///	This is required.
	void set_root_object(cgv::base::node_ptr _root_obj);
	/// Sets the object that defines the coordinate system relative to which the manipulated value is defined.
	///	In most cases this will be the object holding the manipulated value.
	///	This is required.
	void set_value_object(cgv::base::node_ptr _value_obj);

	/// Set a fixed position offset that will be added to the gizmo's position (in anchor coordinate system).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_position(vec3 _offset_position);
	/// Set a fixed rotation offset that will be added to the gizmo's rotation (in anchor coordinate system).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_rotation(quat _offset_rotation);
	/// Set a variable (readonly) position offset that will be added to the gizmo's position (in anchor coordinate system).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_position(const vec3* _offset_position_ptr);
	/// Set a variable (readonly) rotation offset that will be added to the gizmo's rotation (in anchor coordinate system).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_rotation(const quat* _offset_rotation_ptr);
	/// Set a variable (readonly) position offset with an additional level of indirection that will be added to the gizmo's position (in anchor coordinate system).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_position(const vec3** _offset_position_ptr_ptr);
	/// Set a variable (readonly) rotation offset with an additional level of indirection that will be added to the gizmo's rotation (in anchor coordinate system).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_rotation(const quat** _offset_rotation_ptr_ptr);

	/// Set a fixed position offset that will be added to the gizmo's position (in root coordinate system).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_root_offset_position(vec3 _offset_position);
	/// Set a fixed rotation offset that will be added to the gizmo's rotation (in root coordinate system).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_root_offset_rotation(quat _offset_rotation);
	/// Set a variable (readonly) position offset that will be added to the gizmo's position (in root coordinate system).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_root_offset_position(const vec3* _offset_position_ptr);
	/// Set a variable (readonly) rotation offset that will be added to the gizmo's rotation (in root coordinate system).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_root_offset_rotation(const quat* _offset_rotation_ptr);
	/// Set a variable (readonly) position offset with an additional level of indirection that will be added to the gizmo's position (in root coordinate system).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_root_offset_position(const vec3** _offset_position_ptr_ptr);
	/// Set a variable (readonly) rotation offset with an additional level of indirection that will be added to the gizmo's rotation (in root coordinate system).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_root_offset_rotation(const quat** _offset_rotation_ptr_ptr);

	/// Set whether this gizmo's anchor object's transform will change with the value manipulated by this gizmo.
	/// It is important to set this correctly for certain behaviours to work as expected.
	void set_is_anchor_influenced_by_gizmo(bool value);
	/// Set whether this gizmo's root object's transform will change with the value manipulated by this gizmo.
	/// It is important to set this correctly for certain behaviours to work as expected.
	void set_is_root_influenced_by_gizmo(bool value);
	/// Set whether this gizmo's rotation should be based on that of the root object (as opposed to that of the anchor object).
	///	The default value is false.
	void set_use_root_for_rotation(bool value);
	/// Set whether this gizmo's position should be based on that of the root object (as opposed to that of the anchor object).
	///	The default value is false.
	void set_use_root_for_position(bool value);
	///// Set whether the rotation offset should be applied within the coordinate system of the root object (as opposed to that of the anchor object).
	/////	The default value is false.
	//void set_use_root_for_offset_rotation(bool value);
	///// Set whether the position offset should be applied within the coordinate system of the root object (as opposed to that of the anchor object).
	/////	The default value is false.
	//void set_use_root_for_offset_position(bool value);
protected:
	/// Set the object to be notified of value changes. Should be called in subclasses of gizmo when setting the pointer to the manipulated value.
	void set_on_set_object(cgv::base::base_ptr _on_set_obj);
public:
	//@name cgv::base::base interface
	//@{
	std::string get_type_name() const override { return "gizmo2"; }
	//@}

	//@name cgv::nui::focusable interface
	//@{
	bool focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info) override;
	bool handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request) override;
	//@}

	//@name cgv::nui::grabable interface
	//@{
	bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) override;
	//@}
	//@name cgv::nui::pointable interface
	//@{
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) override;
	//@}

	//@name cgv::render::drawable interface
	//@{
	void draw(cgv::render::context& ctx) override;
	void finish_draw(context& ctx) override;
	//@}

	//@name cgv::nui::transforming interface
	//@{
	mat4 get_model_transform() const override;
	//@}
};

	}
}

#include <cgv/config/lib_end.h>