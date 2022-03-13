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
	vec3 position;
	quat rotation;

	vec3 query_point_at_grab, position_at_grab;
	vec3 hit_point_at_trigger, position_at_trigger;

	void on_grabbed_start(vec3 query_point) override;
	void on_grabbed_drag(vec3 query_point) override;
	void on_triggered_start(vec3 hit_point) override;
	void on_triggered_drag(vec3 ray_origin, vec3 ray_direction, vec3 hit_point) override;

public:
	grabable_interactable(const vec3& position, const quat& rotation = quat(1, 0, 0, 0), const std::string& name = "") :
		interactable(name), position(position), rotation(rotation) {}

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