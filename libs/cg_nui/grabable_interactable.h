#pragma once

#include <cg_nui/interactable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

	/// Base class for interactable objects that can be moved while triggered/grabbed. The movement (and rotation)
	///	is applied to variables provided in the constructor as either a pointer or a double pointer.
	///	The double pointer can be used for an additional indirection to support the use of multiple primitives
	///	(see simple_primitive_container in vr_lab_test for an example of this).
	///	If enabled an additional debug point is drawn for the intersection/closest surface point at the start of
	///	grabbing/triggering.
	class CGV_API grabable_interactable : public interactable
{
	bool debug_point_enabled{ false };
	rgb debug_point_color{ rgb(0.4f, 0.05f, 0.6f) };

protected:
	vec3* position_ptr;
	vec3** position_ptr_ptr;
	// TODO: Implement change of rotation (with option to set rotation to nullptr if not needed)
	quat* rotation_ptr;
	quat** rotation_ptr_ptr;

	vec3 query_point_at_grab, position_at_grab;

	void on_grabbed_start(vec3 query_point) override;
	void on_grabbed_drag(vec3 query_point) override;
	void on_triggered_start(vec3 hit_point) override;
	void on_triggered_drag(vec3 ray_origin, vec3 ray_direction, vec3 hit_point) override;

public:
	grabable_interactable(vec3* position_ptr, quat* rotation_ptr, const std::string& name = "") :
		interactable(name), position_ptr(position_ptr), position_ptr_ptr(nullptr), rotation_ptr(rotation_ptr), rotation_ptr_ptr(nullptr) {}

	grabable_interactable(vec3* position_ptr, const std::string& name = "") :
		interactable(name), position_ptr(position_ptr), position_ptr_ptr(nullptr), rotation_ptr(nullptr), rotation_ptr_ptr(nullptr) {}

	grabable_interactable(quat* rotation_ptr, const std::string& name = "") :
		interactable(name), position_ptr(nullptr), position_ptr_ptr(nullptr), rotation_ptr(rotation_ptr), rotation_ptr_ptr(nullptr) {}

	grabable_interactable(vec3** position_ptr_ptr, quat** rotation_ptr_ptr, const std::string& name = "") :
		interactable(name), position_ptr(nullptr), position_ptr_ptr(position_ptr_ptr), rotation_ptr(nullptr), rotation_ptr_ptr(rotation_ptr_ptr) {}

	grabable_interactable(vec3** position_ptr_ptr, const std::string& name = "") :
		interactable(name), position_ptr(nullptr), position_ptr_ptr(position_ptr_ptr), rotation_ptr(nullptr), rotation_ptr_ptr(nullptr) {}

	grabable_interactable(quat** rotation_ptr_ptr, const std::string& name = "") :
		interactable(name), position_ptr(nullptr), position_ptr_ptr(nullptr), rotation_ptr(nullptr), rotation_ptr_ptr(rotation_ptr_ptr) {}

	// Used for drawing debug points
	//@name cgv::render::drawable interface
	//@{
	void draw(cgv::render::context& ctx) override;
	//@}

	// Used for debug settings
	//@name cgv::gui::provider interface
	//@{
	void create_gui() override;
	//@}
};
	}
}