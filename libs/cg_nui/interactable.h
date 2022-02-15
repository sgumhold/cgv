#pragma once

#include <cgv/base/group.h>
#include <cgv/render/drawable.h>
#include <cg_nui/focusable.h>
#include <cg_nui/pointable.h>
#include <cg_nui/grabable.h>
#include <cgv/gui/provider.h>
#include <cgv_gl/sphere_renderer.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// Base class for objects that can have focus and be selected/activated by pointing or grabbing.
///	Provides a general implementation of different interaction states with event functions that can be overriden.
class CGV_API interactable : public cgv::base::group,
					  public cgv::render::drawable,
					  public cgv::nui::focusable,
					  public cgv::nui::grabable,
					  public cgv::nui::pointable,
					  public cgv::gui::provider
{
	cgv::nui::focus_configuration original_config;
protected:
	vec3 debug_point;
	rgb debug_point_color;
	cgv::render::sphere_render_style srs;

	vec3 cached_query_point;
	vec3 cached_hit_point;

public:
	// different possible object states
	enum class state_enum { idle, close, pointed, grabbed, triggered };

private:
	void change_state(state_enum new_state, vec3 query_point = vec3(0.0));

protected:
	// hid with focus on object
	cgv::nui::hid_identifier hid_id;
	// state of object
	state_enum state = state_enum::idle;

	// State change events that can be overriden

	virtual void on_close_start() {}
	virtual void on_close_stop() {}

	virtual void on_pointed_start() {}
	virtual void on_pointed_stop() {}

	virtual void on_grabbed_start(vec3 query_point) {}
	virtual void on_grabbed_drag(vec3 query_point) {}
	virtual void on_grabbed_stop() {}

	virtual void on_triggered_start(vec3 hit_point) {}
	virtual void on_triggered_drag(vec3 ray_origin, vec3 ray_direction, vec3 hit_point) {}
	virtual void on_triggered_stop() {}

public:
	interactable(const std::string& name = "");

	//@name cgv::base::base interface
	//@{
	std::string get_type_name() const override { return "interactable"; }
	void on_set(void* member_ptr) override;
	//@}

	//@name cgv::nui::focusable interface
	//@{
	bool focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa,
					  const cgv::nui::focus_demand& demand, const cgv::gui::event& e,
					  const cgv::nui::dispatch_info& dis_info) override;
	void stream_help(std::ostream& os) override;
	bool handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request) override;
	//@}

	// Used for drawing debug points
	//@name cgv::render::drawable interface
	//@{
	bool init(cgv::render::context& ctx) override;
	void clear(cgv::render::context& ctx) override;
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