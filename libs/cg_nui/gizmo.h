#pragma once

#include <cg_nui/interactable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// Abstract base class for gizmos.
///	A gizmo has an attach function that takes at least a base_ptr to the object this gizmo attaches to. The base_ptr is
///	used to call the on_set function when the gizmo modifies values.
///	Specific gizmo subclasses take additional pointers to to-be-manipulated values as arguments to attach.
///	A gizmo also has a detach function that clears the connection between object and gizmo.
class CGV_API gizmo : public cgv::nui::interactable
{
	/// Readonly position this gizmo is anchored to (e.g. position of an object)
	const vec3* anchor_position_ptr;
	/// Optional readonly rotation this gizmo is anchored to (e.g. rotation of an object) only needed
	///	if the gizmo operates in a local coordinate system (e.g. the visual representation of the gizmo
	///	follows the object's rotation).
	const quat* anchor_rotation_ptr;
	/// Optional readonly scale this gizmo is anchored to only needed if the gizmo's visual representation
	///	should change with the objects scale.
	const vec3* anchor_scale_ptr;

protected:
	base_ptr obj_ptr;
	bool is_attached = false;

	// used to track if recomputation of geometry is needed
	vec3 current_anchor_position{ vec3(0.0) };
	quat current_anchor_rotation{ quat() };
	vec3 current_anchor_scale{ vec3(1.0) };

	// Needed to call the two events on_handle_grabbed and on_handle_drag
	void on_grabbed_start() override
	{
		on_handle_grabbed();
	}

	void on_grabbed_drag() override
	{
		on_handle_drag();
	}

	void on_grabbed_stop() override
	{
		on_handle_released();
	}

	void on_triggered_start() override
	{
		on_handle_grabbed();
	}

	void on_triggered_drag() override
	{
		on_handle_drag();
	}

	void on_triggered_stop() override
	{
		on_handle_released();
	}

	/// Event that is called when a primitive/handle of the gizmo gets grabbed by a hid.
	///	prim_idx is the primitive that was grabbed, start_position is the point it was grabbed at.
	virtual void on_handle_grabbed() {}
	/// Event that is called whenever the hid moves/rotates while grabbing the gizmo.
	///	prim_idx is the primitive that was grabbed, start_position is the point it was grabbed at,
	///	target_position is the start_position projected to reflect the movement of the hid.
	virtual void on_handle_drag() {}

	/// Event that is called when a primitive/handle of the gizmo gets released from grabbing by a hid.
	virtual void on_handle_released() {}

	// Update the gizmo's geometry for the current anchor position and rotation
	virtual void compute_geometry() = 0;

public:
	gizmo(const std::string& name = "") : interactable(name) {}

	void attach(base_ptr obj, const vec3* anchor_position_ptr, const quat* anchor_rotation_ptr = nullptr, const vec3* anchor_scale_ptr = nullptr)
	{
		obj_ptr = obj;
		is_attached = true;
		if (anchor_position_ptr != nullptr)
			this->anchor_position_ptr = anchor_position_ptr;
		else
			this->anchor_position_ptr = new vec3(0.0);
		if (anchor_rotation_ptr != nullptr)
			this->anchor_rotation_ptr = anchor_rotation_ptr;
		else
			this->anchor_rotation_ptr = new quat();
		if (anchor_scale_ptr != nullptr)
			this->anchor_scale_ptr = anchor_scale_ptr;
		else
			this->anchor_scale_ptr = new vec3(1.0);
		current_anchor_position = *this->anchor_position_ptr;
		current_anchor_rotation = *this->anchor_rotation_ptr;
		current_anchor_scale = *this->anchor_scale_ptr;
		compute_geometry();
	}

	void detach()
	{
		if (!is_attached)
			return;
		is_attached = false;
		obj_ptr.clear();
	}


	//@name cgv::base::base interface
	//@{
	std::string get_type_name() const override
	{
		return "gizmo";
	}
	//@}

	//@name cgv::nui::focusable interface
	//@{
	bool focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info) override
	{
		if (!is_attached)
			return false;
		return interactable::focus_change(action, rfa, demand, e, dis_info);
	}
	bool handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request) override
	{
		if (!is_attached)
			return false;
		return interactable::handle(e, dis_info, request);
	}
	//@}

	//@name cgv::render::drawable interface
	//@{
	void draw(cgv::render::context& ctx) override
	{
		if (!is_attached)
			return;
		// Check if geometry has to be updated
		if (current_anchor_position != *anchor_position_ptr ||
			current_anchor_rotation != *anchor_rotation_ptr ||
			current_anchor_scale != *anchor_scale_ptr)
		{
			compute_geometry();
			current_anchor_position = *anchor_position_ptr;
			current_anchor_rotation = *anchor_rotation_ptr;
			current_anchor_scale = *anchor_scale_ptr;
		}
	}
	//@}

protected:
	vec3 relative_to_absolute_position(vec3 relative_position, bool consider_local_rotation = true, bool ignore_scale = false)
	{
		vec3 scale = ignore_scale ? vec3(1.0) : current_anchor_scale;
		if (consider_local_rotation)
			return current_anchor_position + current_anchor_rotation.apply(relative_position * scale);
		else
			return current_anchor_position + relative_position * scale;
	}

	vec3 relative_to_absolute_direction(vec3 relative_direction, bool consider_local_rotation = true)
	{
		if (consider_local_rotation)
			return current_anchor_rotation.apply(relative_direction);
		else
			return relative_direction;
	}
};

	}
}

#include <cgv/config/lib_end.h>