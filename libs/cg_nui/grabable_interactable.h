#pragma once

#include <cg_nui/interactable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {


class CGV_API grabable_interactable : public interactable
{
	rgb debug_point_grab_color;
	rgb debug_point_trigger_color;

protected:
	vec3* position_ptr;
	vec3** position_ptr_ptr;
	// TODO: Implement change of rotation (with option to set rotation to nullptr if not needed)
	quat* rotation_ptr;
	quat** rotation_ptr_ptr;

	vec3 query_point_at_grab, position_at_grab;
	vec3 hit_point_at_trigger, position_at_trigger;

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

	// To be implemented by deriving classes
	//@name cgv::nui::grabable interface
	//@{
	bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) override = 0;
	//@}
	//@name cgv::nui::pointable interface
	//@{
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) override = 0;

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