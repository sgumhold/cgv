#pragma once

#include <cg_nui/grabable_interactable.h>

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
protected:
	base_ptr obj_ptr;
	bool is_attached = false;
	/// Readonly position this gizmo is anchored to (e.g. position of an object)
	const vec3* anchor_position_ptr;
	/// Optional readonly rotation this gizmo is anchored to (e.g. rotation of an object) only needed
	///	if the gizmo operates in a local coordinate system (e.g. the visual representation of the gizmo
	///	follows the object's rotation).
	const quat* anchor_rotation_ptr;

	// Needed to call the two events on_handle_grabbed and on_handle_drag
	void on_grabbed_start() override
	{
		on_handle_grabbed();
	}

	void on_grabbed_drag() override
	{
		on_handle_drag();
	}

	void on_triggered_start() override
	{
		on_handle_grabbed();
	}

	void on_triggered_drag() override
	{
		on_handle_drag();
	}

	/// Event that is called when a primitive/handle of the gizmo gets grabbed by a hid.
	///	prim_idx is the primitive that was grabbed, start_position is the point it was grabbed at.
	virtual void on_handle_grabbed() {}
	/// Event that is called whenever the hid moves/rotates while grabbing the gizmo.
	///	prim_idx is the primitive that was grabbed, start_position is the point it was grabbed at,
	///	target_position is the start_position projected to reflect the movement of the hid.
	virtual void on_handle_drag() {}

public:
	gizmo(const std::string& name = "") : interactable(name) {}

	void attach(base_ptr obj, const vec3* anchor_position_ptr, const quat* anchor_rotation_ptr = nullptr)
	{
		obj_ptr = obj;
		is_attached = true;
		this->anchor_position_ptr = anchor_position_ptr;
		this->anchor_rotation_ptr = anchor_rotation_ptr;
	}

	void detach()
	{
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
};

	}
}

#include <cgv/config/lib_end.h>