#pragma once

#include <cg_nui/interactable.h>
#include <cg_nui/translatable.h>
#include <cg_nui/transforming.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// Base class for interactable objects that can be moved while triggered/grabbed. The movement (and rotation)
///	is applied via the translatable (and rotatable) interfaces. These have to be implemented by the object
///	implementing this interface. (This interface does not inherit them itself to avoid virtual inheritance.)
///	(See simple_object and simple_primitive_container in vr_lab_test for examples of using this interface.)
class CGV_API poseable : public interactable
{
	transforming* _transforming{ nullptr };
	bool tried_transforming_cast{ false };
	transforming* get_transforming();

	translatable* _translatable{ nullptr };
	bool tried_translatable_cast{ false };
	translatable* get_translatable();

	//rotatable* _rotatable{ nullptr };
	//bool tried_rotatable_cast{ false };
	//rotatable* get_rotatable();
protected:
	// TODO: Implement change of rotation

	// TODO: Probably remove as they are not used
	vec3 position_at_grab;
	quat rotation_at_grab;

	bool manipulate_rotation{ true };

	void on_grabbed_start() override;
	void on_grabbed_drag() override;
	void on_triggered_start() override;
	void on_triggered_drag() override;

public:
	poseable(const std::string& name = "", bool manipulate_rotation = true) : interactable(name), manipulate_rotation(manipulate_rotation) {}

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
