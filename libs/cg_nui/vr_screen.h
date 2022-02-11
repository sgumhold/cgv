#include <memory>
#include <cg_nui/focusable.h>
#include <cg_nui/pointable.h>
#include <cg_nui/transformed.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv/gui/provider.h>
#include <cgv/render/drawable.h>
#include <cgv/base/node.h>
#include <cg_vr/vr_events.h>

namespace SL {
	namespace Screen_Capture {
		class IScreenCaptureManager;
		struct Image;
		struct Monitor;
	}
}

#include "lib_begin.h"

namespace cgv {
	namespace nui {

/// class manages static and dynamic parts of scene
class CGV_API vr_screen :
	public cgv::base::node,
	public cgv::render::drawable,
	public cgv::nui::focusable,
	public cgv::nui::pointable,
	public cgv::nui::transformed,
	public cgv::gui::provider
{
protected:
	// screen rendering
	bool show_screen = false;
	float screen_aspect = 1.333f;
	cgv::render::rectangle_render_style screen_style;
	cgv::render::texture screen_texture;
	vec3 screen_center;
	vec2 screen_extent;
	quat screen_rotation;

	/// possible states of screen
	enum class state_enum {
		idle = 0,  // no input focus
		place,     // kit focus grabed for hmd+ctrl based placement of screen
		mouse,     // controller focus for emulation of mouse input
		drag       // controller focus in emulation during drag operation
	};
	/// current state of screen
	state_enum state = state_enum::idle;

	cgv::nui::focus_configuration original_config;

	// screen placement
	vec3 screen_reference;
	float screen_distance = 2.0f;
	float screen_scale = 1.0f;
	float screen_x_offset = 0.0f;
	void place_screen(); // place center and extent based on reference, rotation, distance, scale and x_offset
private:
	bool last_show_screen = false;
protected:
	bool initial_placement = true;
	double start_placement_time;
	mat34 start_placement_pose;
	bool place_screen(const vr::vr_kit_state& state); // define reference, rotation, distance, scale and x_offset based on hmd and placement controller

	// screen capture support
	bool screen_capture;
	std::vector<bool> screen_image_dirty;
	std::vector<cgv::data::data_format> screen_image_dfs;
	std::vector<cgv::data::data_view> screen_images;

	std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> screen_capture_manager;
	void screen_capture_callback(const SL::Screen_Capture::Image& img, const SL::Screen_Capture::Monitor& monitor);

	// mouse emulation
	cgv::nui::hid_identifier hid_id;
	vec3 screen_point;
	bool mouse_button_pressed[3];
	void compute_mouse_xy(const vec3& p, int& X, int& Y) const;

	// whether screen is enabled
	bool enabled;

	bool compute_intersection(const vec3& rectangle_center, const vec2& rectangle_extent, const vec3& ray_start, const vec3& ray_direction, float& ray_param);
	bool compute_intersection(const vec3& rectangle_center, const vec2& rectangle_extent, const quat& rectangle_rotation, const vec3& ray_start, const vec3& ray_direction, float& ray_param);
	void placement_trigger(const cgv::gui::vr_key_event& vrke, cgv::nui::focus_request& request);
public:
	/** node interface*/
	//@{
	/// standard constructor for scene
	vr_screen(const std::string& name);
	/// return type name
	std::string get_type_name() const { return "vr_screen"; }
	/// reflect member variables
	bool self_reflect(cgv::reflect::reflection_handler& rh);
	/// callback on member updates to keep data structure consistent
	void on_set(void* member_ptr);
	//@}

	/** focusable interface*/
	//@{
	/// ask focusable what it wants to grab focus based on given event - in case of yes, the focus_demand should be filled
	bool wants_to_grab_focus(const cgv::gui::event& e, const cgv::nui::hid_identifier& hid_id, cgv::nui::focus_demand& demand);
	/// inform focusable that its focus changed (fst param tells whether receiving or loosing) together with focus request and causing event plus dispatch info
	bool focus_change(cgv::nui::focus_change_action action, cgv::nui::refocus_action rfa, const cgv::nui::focus_demand& demand, const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info);
	/// stream help information to the given output stream
	void stream_help(std::ostream& os);
	//! ask active focusable to handle event providing dispatch info
	/*! return whether event was handled
		To request a focus change, fill the passed request struct and set the focus_change_request flag.*/
	bool handle(const cgv::gui::event& e, const cgv::nui::dispatch_info& dis_info, cgv::nui::focus_request& request);
	//@}
	/** pointable interface */
	//@{
	/// implement ray object intersection and return whether intersection was found and in this case set \c hit_param to ray parameter and optionally \c hit_normal to surface normal of intersection
	bool compute_intersection(const vec3& ray_start, const vec3& ray_direction, float& hit_param, vec3& hit_normal, size_t& primitive_idx);
	//@}

	/** drawable interface*/
	//@{
	/// initialization called once per context creation
	bool init(cgv::render::context& ctx);
	/// initialization called once per frame
	void init_frame(cgv::render::context& ctx);
	/// called before context destruction to clean up GPU objects
	void clear(cgv::render::context& ctx);
	/// draw scene here
	void draw(cgv::render::context& ctx);
	//@}

	/** provider interface*/
	//@{
	/// cgv::gui::provider function to create classic UI
	void create_gui();
	//@}
};

typedef cgv::data::ref_ptr<vr_screen, true> vr_screen_ptr;

	}
}
#include <cgv/config/lib_end.h>