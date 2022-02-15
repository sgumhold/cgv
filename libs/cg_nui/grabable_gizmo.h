#pragma once

#include <cg_nui/gizmo.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// A gizmo that enables an object to be grabbed and moved around following the vr controller.
///	It has no visual representation and requires the attached object to be an interactable for intersection and closest point calculations.
class CGV_API grabable_gizmo : public cgv::nui::gizmo
{
	rgb debug_point_grab_color;
	rgb debug_point_trigger_color;
public:
	typedef std::function<bool(const vec3&, vec3&, vec3&, size_t&)> compute_closest_point_callback_t;
	typedef std::function<bool(const vec3&, const vec3&, float&, vec3&, size_t&)> compute_intersection_callback_t;
protected:
	compute_closest_point_callback_t compute_closest_point_callback;
	compute_intersection_callback_t compute_intersection_callback;
	vec3* position_member_ptr;

	vec3 query_point_at_grab, position_at_grab;
	vec3 hit_point_at_trigger, position_at_trigger;

	void on_grabbed_start(vec3 query_point) override;
	void on_grabbed_drag(vec3 query_point) override;
	void on_triggered_start(vec3 hit_point) override;
	void on_triggered_drag(vec3 ray_origin, vec3 ray_direction, vec3 hit_point) override;

public:
	grabable_gizmo(const std::string& name = "grabbable_gizmo") : gizmo(name),
		compute_closest_point_callback([](const vec3& a, vec3& b, vec3& c, size_t& d) { return false; }),
		compute_intersection_callback([](const vec3& a, const vec3& b, float& c, vec3& d, size_t& e) { return false; }),
		position_member_ptr(nullptr), debug_point_grab_color(rgb(0.5f, 0.5f, 0.5f)), debug_point_trigger_color(rgb(0.3f, 0.3f, 0.3f)){}

	void attach(compute_closest_point_callback_t compute_closest_point_callback,
		compute_intersection_callback_t compute_intersection_callback, void* position_member);

	void detach();


	//@name cgv::base::base interface
	//@{
	std::string get_type_name() const override
	{
		return "gizmo";
	}
	//bool self_reflect(cgv::reflect::reflection_handler& rh) override;
	//void on_set(void* member_ptr) override;
	//@}

	//@name cgv::nui::focusable interface
	//@{
	void stream_help(std::ostream& os) override;
	//@}

	//@name cgv::nui::grabable interface
	//@{
	bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx) override;
	//@}

	//@name cgv::nui::pointable interface
	//@{
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx) override;
	//@}

	// Used for drawing debug points
	//@name cgv::render::drawable interface
	//@{
	void draw(cgv::render::context& ctx) override;
	//@}

	//@name cgv::gui::provider interface
	//@{
	void create_gui() override;
	//@}
};

	}
}

#include <cgv/config/lib_end.h>