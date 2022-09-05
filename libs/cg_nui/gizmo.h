#pragma once

#include <cg_nui/interactable.h>
#include <cg_nui/transforming.h>
#include <cgv/math/ftransform.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {
/// Abstract base class for gizmos.
///	A gizmo has an attach function that takes at least a base_ptr to the object this gizmo attaches to. The base_ptr is
///	used to call the on_set function when the gizmo modifies values.
///	Specific gizmo subclasses take additional pointers to to-be-manipulated values as arguments to attach.
///	A gizmo also has a detach function that clears the connection between object and gizmo.
class CGV_API gizmo : public cgv::nui::interactable, public cgv::nui::transforming
{
protected:
	/// Fixed position offset to the anchor position (added to position of anchor object). Do not use directly (use through anchor_position_ptr).
	vec3 anchor_position;
	/// Fixed rotation offset to the anchor rotation (added to rotation of anchor object). Do not use directly (use through anchor_rotation_ptr).
	quat anchor_rotation;
	/// Readonly position offset to the anchor position (added to position of anchor object)
	const vec3* anchor_position_ptr{ nullptr };
	/// Readonly rotation offset to the anchor rotation (added to rotation of anchor object)
	const quat* anchor_rotation_ptr{ nullptr };
	/// Readonly position offset to the anchor position (added to position of anchor object)
	const vec3** anchor_position_ptr_ptr{ nullptr };
	/// Readonly rotation offset to the anchor rotation (added to rotation of anchor object)
	const quat** anchor_rotation_ptr_ptr{ nullptr };
	/// Reference of object this gizmo is anchored to
	cgv::base::node_ptr anchor_obj{ nullptr };

protected:
	bool is_attached{ false };
	bool use_absolute_rotation{ false };
	bool is_anchor_influenced_by_gizmo{ false };

	/// Reference to the object that gets notified of changing values through the on_set function
	cgv::base::base_ptr on_set_obj{ nullptr };

	// Needed to call the two events on_handle_grabbed and on_handle_drag
	void on_grabbed_start() override { on_handle_grabbed();	}

	void on_grabbed_drag() override	{ on_handle_drag();	}

	void on_grabbed_stop() override	{ on_handle_released();	}

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
	virtual bool validate_configuration() = 0;

	/// Compute constant values according to the configuration.
	/// Called once on attach.
	virtual void precompute_geometry() = 0;

	/// Internal draw function in local coordinate system with scale (and possibly rotation) removed that has access to the camera transform
	virtual void _draw(cgv::render::context& ctx, const vec3& scale, const mat4& view_matrix) {}

	/// Internal compute_closest_point function in local coordinate system with scale (and possibly rotation) removed that has access to the camera transform
	virtual bool _compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx, const vec3& scale,
		const mat4& view_matrix) { return false; }
	/// Internal compute_intersection function in local coordinate system with scale (and possibly rotation) removed that has access to the camera transform
	virtual bool _compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx,
		const vec3& scale, const mat4& view_matrix) { return false; }

private:
	/// Compute transform that removes the anchor object's global scale (and possibly rotation) from the model-view matrix.
	mat4 compute_draw_correction_transformation(vec3& scale);
	/// Compute transform that removes the anchor object's global scale (and possibly rotation) from the parameters of the intersection/proximity functions.
	mat4 compute_interaction_correction_transformation(vec3& scale);

public:
	gizmo(const std::string& name = "") : interactable(name) {}

	/// Validates current configuration and activates gizmo if correct.
	void attach();
	/// Deactivates gizmo
	void detach();

	// Configuration Functions

	/// Sets the object as the parent of the gizmo. The gizmo will be positioned according to any transforming interfaces in the hierarchy.
	///	Not setting this means the gizmo is positioned in the global coordinate system directly.
	void set_anchor_object(cgv::base::node_ptr _anchor_obj);
	/// Set a fixed position offset that will be added to the anchor object's position (if object was set).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_position(vec3 _anchor_position);
	/// Set a fixed rotation offset that will be added to the anchor object's rotation (if object was set).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_rotation(quat _anchor_rotation);
	/// Set a variable (readonly) position offset that will be added to the anchor object's position (if object was set).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_position(const vec3* _anchor_position_ptr);
	/// Set a variable (readonly) rotation offset that will be added to the anchor object's rotation (if object was set).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_rotation(const quat* _anchor_rotation_ptr);
	/// Set a variable (readonly) position offset with an additional level of indirection that will be added to the anchor object's position (if object was set).
	///	This will only be used to position the gizmo and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_position(const vec3** _anchor_position_ptr_ptr);
	/// Set a variable (readonly) rotation offset with an additional level of indirection that will be added to the anchor object's rotation (if object was set).
	///	This will only be used to position the gizmo's handles and is NOT a value being manipulated by the gizmo.
	void set_anchor_offset_rotation(const quat** _anchor_rotation_ptr_ptr);
	/// Set whether the gizmo should be rotated relative to the world as opposed to relative to its anchor object.
	///	The anchor rotation (if set) will be applied regardless.
	void set_use_absolute_rotation(bool value);
	/// Set whether this gizmos anchor will change with the value manipulated by this gizmo.
	/// It is important to set this correctly for certain behaviours to work as expected.
	void set_is_anchor_influenced_by_gizmo(bool value);
protected:
	/// Set the object to be notified of value changes. Should be called in subclasses of gizmo when setting the pointer to the manipulated value.
	void set_on_set_object(cgv::base::base_ptr _on_set_obj);
public:
	//@name cgv::base::base interface
	//@{
	std::string get_type_name() const override { return "gizmo"; }
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
	//@}

	//@name cgv::nui::transforming interface
	//@{
	const mat4& get_model_transform() const override;
	const mat4& get_inverse_model_transform() const override;
	vec3 get_local_position() const override;
	quat get_local_rotation() const override;
	vec3 get_local_scale() const override;
	//@}
};

	}
}

#include <cgv/config/lib_end.h>