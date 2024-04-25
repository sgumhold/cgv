#pragma once

#include <cgv/base/node.h>
#include <cgv/render/drawable.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/box_wire_renderer.h>
#include <cgv_gl/cone_renderer.h>
#include <cgv/gui/provider.h>
#include <cg_nui/focusable.h>
#include <cg_nui/grabbable.h>
#include <cg_nui/pointable.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// different table types
enum table_mode
{
	rectangular,
	round
};

class table_gizmo;

/// support self reflection of table mode
extern CGV_API cgv::reflect::enum_reflection_traits<table_mode> get_reflection_traits(const table_mode&);

/// class manages static and dynamic parts of scene
class CGV_API vr_table :
	public cgv::base::group,
	public cgv::render::drawable,
	public cgv::nui::focusable,
	public cgv::nui::grabbable,
	public cgv::nui::pointable,
	public cgv::gui::provider
{
protected:
	friend class table_gizmo;
	cgv::data::ref_ptr<table_gizmo, true> gizmo;
	bool use_gizmo;
	//@name interaction
	//@{
	/// temporary storage for focus config before restriction during grab
	cgv::nui::focus_configuration original_config;
	/// possible states of vr_table
	enum class state_enum {
		idle=0,  // no input focus
		close,   // input focus due to proximity
		pointed, // input focus due to intersection
		grabbed, // grabbed in proximity based focus
		gizmo    // gizmo activated during pointing 
	};
	/// current state of table
	state_enum state = state_enum::idle;
	/// focus hid (includes controller index)
	cgv::nui::hid_identifier hid_id;
	/// query point at grab
	vec3 query_point_at_grab;
	/// controlled values at grab
	float scale_at_grab;
	float height_at_grab;
	float angle_at_grab;
	/// update scale, height and angle from position at grab and current position
	void place_table(const vec3& p0, const vec3& p1);
	//@}

	//@name geometric representations of boxes table
	//@{	

	// store the static part of the scene as colored boxes with the table in the last 5 boxes
	std::vector<box3> boxes;
	std::vector<rgb> box_colors;

	// rendering style for rendering of boxes
	cgv::render::box_render_style box_style;

	// use cones for the turn table
	std::vector<vec4> cone_vertices;
	std::vector<rgb> cone_colors;

	// rendering style for rendering of cones
	cgv::render::cone_render_style cone_style;
	cgv::render::cone_renderer cone_renderer;

	// rendering style for rendering of boxes
	cgv::render::box_wire_render_style box_wire_style;

	/**@name ui parameters for table construction*/
	//@{
	/// table mode
	table_mode mode = table_mode::round;
	/// width in rectangular and radius in round mode
	float scale = 1.6f;
	/// rotation angle
	float angle = 0;
	/// aspect ratio in rectangular and fraction of top over bottom radius in round mode
	float aspect = 2.0f;
	/// height of table measured from ground to top face
	float height = 0.7f;
	/// width of legs of rectangular table or radius of central leg of round table
	float leg_width = 0.03f;
	/// offset of legs relative to table width/radius 
	float percentual_leg_offset = 0.03f;
	/// color of table top and legs
	rgb table_color, leg_color;
	/// show unit box [-0.5,0,-0.5]-[0.5,1,0.5] as wireframe
	bool show_wirebox = false;
	/// color of wirebox
	rgb wire_color;
	/// return table color that depends on focus and grab state
	rgb get_table_color() const;
	//@}

	/// construct boxes that represent a rectangular table of dimensions tw,td,th, leg width tW, percentual leg offset and table/leg colors
	void construct_rectangular_table(float tw, float td, float th, float tW, float tpO, rgb table_clr, rgb leg_clr);
	/// construct cones that represent a round table of dimensions top/bottom radius ttr/tbr, height th, leg width tW, percentual leg offset and table/leg colors
	void construct_round_table(float ttr, float tbr, float th, float tW, float tpO, rgb table_clr, rgb leg_clr);
	/// construct a scene with a table
	void build_table();
	/// change highlightable table color after table construction
	void update_table_color(const rgb& color);
	//@}
	/// implement closest point algorithm and return whether this was found (failure only for invisible objects) and in this case set \c prj_point to closest point and \c prj_normal to corresponding surface normal
	bool compute_closest_point(const vec3& point, vec3& prj_point, vec3& prj_normal, size_t& primitive_idx);
	/// implement ray object intersection and return whether intersection was found and in this case set \c hit_param to ray parameter and optionally \c hit_normal to surface normal of intersection
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx);
public:
	/// standard constructor for scene
	vr_table();
	/// 
	mat4 get_transform() const;
	/// 
	mat3 get_normal_transform() const;
	/// return type name
	std::string get_type_name() const { return "vr_table"; }
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

	//@name cgv::render::drawable interface
	//@{
	/// initialization called once per context creation
	bool init(cgv::render::context& ctx);
	/// called before context destruction to clean up GPU objects
	void clear(cgv::render::context& ctx);
	/// draw scene here
	void draw(cgv::render::context& ctx);
	//@}
	/// provide access to table dimensions
	vec3 get_table_extent() const { return vec3(scale, height, scale / aspect); }
	/// cgv::gui::provider function to create classic UI
	void create_gui();
};

typedef cgv::data::ref_ptr<vr_table, true> vr_table_ptr;

	}
}

#include <cgv/config/lib_end.h>