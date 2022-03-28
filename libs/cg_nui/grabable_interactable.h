#pragma once

#include <cg_nui/interactable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

	/// Base class for interactable objects that can be moved while triggered/grabbed. The movement (and rotation)
	///	is applied to variables provided in the constructor as either a pointer or a double pointer.
	///	The double pointer can be used for an additional indirection to support the use of multiple primitives
	///	(see simple_primitive_container in vr_lab_test for an example of this).
	class CGV_API grabable_interactable : public interactable
{
protected:
	vec3* position_ptr;
	vec3** position_ptr_ptr;
	// TODO: Implement change of rotation (with option to set rotation to nullptr if not needed)
	quat* rotation_ptr;
	quat** rotation_ptr_ptr;

	vec3 position_at_grab;

	void on_grabbed_start() override;
	void on_grabbed_drag() override;
	void on_triggered_start() override;
	void on_triggered_drag() override;

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