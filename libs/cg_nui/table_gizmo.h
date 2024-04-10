#pragma once

#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/spline_tube_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/arrow_renderer.h>
#include <cgv/gui/provider.h>
#include <cg_nui/focusable.h>
#include <cg_nui/grabbable.h>
#include <cg_nui/pointable.h>
#include "vr_table.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// class manages static and dynamic parts of scene
class CGV_API table_gizmo :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::nui::focusable,
	public cgv::nui::grabbable,
	public cgv::nui::pointable,
	public cgv::gui::provider
{
protected:
	mutable std::vector<vec3> debug_points;
	cgv::render::sphere_render_style srs;

	struct interaction_info {
		bool in_focus = false;
		bool is_triggered = false;
		cgv::nui::focus_configuration original_config;
		cgv::nui::hid_identifier hid_id;
		int arrow_index;
		float value_at_trigger;
		vec3 hit_point_at_trigger;
	};
	std::vector<interaction_info> iis;
	unsigned nr_scale_arrows = 4;
	int scale_arrow_focus_idx = -1;
	void compute_arrow_geometry();
	std::vector<vec3> arrow_positions;
	std::vector<vec3> arrow_directions;
	std::vector<rgb> arrow_colors;

	cgv::render::arrow_render_style ars;

	void compute_spline_geometry(bool in_focus);
	unsigned nr_spline_segments = 4;
	std::vector<vec3> spline_positions;
	std::vector<vec4> spline_tangents;
	float radius = 0.05f;
	float draw_radius_scale = 0.5f;

	cgv::render::spline_tube_render_style strs;

	vr_table_ptr table;
	void update_rotation_angle(float& v, float v0, const vec3& origin, const vec3& p0, const vec3& ro, const vec3& rd) const;
	void update_height(float& v, float v0, const vec3& p0, const vec3& ro, const vec3& rd) const; 
	void update_scale(float& v, float v0, const vec3& p0, const vec3& ad, const vec3& ro, const vec3& rd) const;

public:
	/// standard constructor for scene
	table_gizmo();
	/// 
	void attach(vr_table_ptr _table);
	/// 
	void detach();
	/// return type name
	std::string get_type_name() const { return "table_gizmo"; }
	/// reflect member variables
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// callback on member updates to keep data structure consistent
	void on_set(void* member_ptr);
	//@name cgv::nui::focusable interface
	//@{
	bool focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info);
	void stream_help(std::ostream& os);
	bool handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request);
	//@}

	/// implement closest point algorithm and return whether this was found (failure only for invisible objects) and in this case set \c prj_point to closest point and \c prj_normal to corresponding surface normal
	bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx);
	/// implement ray object intersection and return whether intersection was found and in this case set \c hit_param to ray parameter and optionally \c hit_normal to surface normal of intersection
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx);

	//@name cgv::render::drawable interface
	//@{
	/// initialization called once per context creation
	bool init(cgv::render::context& ctx);
	/// called before context destruction to clean up GPU objects
	void clear(cgv::render::context& ctx);
	/// draw scene here
	void draw(cgv::render::context& ctx);
	//@}
	/// cgv::gui::provider function to create classic UI
	void create_gui();
};

typedef cgv::data::ref_ptr<table_gizmo, true> table_gizmo_ptr;
	}
}

#include <cgv/config/lib_end.h>