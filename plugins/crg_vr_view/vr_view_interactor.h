#pragma once

#include <cgv/base/base.h>
#include <vr/vr_kit.h>
#include <cg_vr/vr_server.h>
#include <vr/vr_driver.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <plugins/crg_stereo_view/stereo_view_interactor.h>

#include "lib_begin.h"

///@ingroup VR
///@{

/// different visualization types for vr kit components
enum VRkitVisType
{
	VVT_NONE,
	VVT_SPHERE = 1,
	VVT_MESH = 2,
	VVT_BOTH
};

/// support reflection of enum names
extern CGV_API cgv::reflect::enum_reflection_traits<VRkitVisType> get_reflection_traits(const VRkitVisType& gm);

//! extends the stereo view interactor for vr support
/*! Besides adding the crg_vr_view plugin to your project, you can configure
    the vr_view_interactor in your cgv::render::drawable::init() function similar
	to the following example:
~~~~~~~~~~~~~~~~~~~~~~~~
#include <vr_view_interactor.h>

bool your_class::init(cgv::render::context& ctx)
{
	cgv::gui::connect_vr_server(true);

	auto view_ptr = find_view_as_node();
	if (view_ptr) {
		// if the view points to a vr_view_interactor
		vr_view_ptr = dynamic_cast<vr_view_interactor*>(view_ptr);
		if (vr_view_ptr) {
			// configure vr event processing
			vr_view_ptr->set_event_type_flags(
				cgv::gui::VREventTypeFlags(
					cgv::gui::VRE_KEY +
					cgv::gui::VRE_THROTTLE+
					cgv::gui::VRE_STICK +
					cgv::gui::VRE_STICK_KEY +
					cgv::gui::VRE_POSE
				));
			vr_view_ptr->enable_vr_event_debugging(false);
			// configure vr rendering
			vr_view_ptr->draw_action_zone(false);
			vr_view_ptr->draw_vr_kits(true);
			vr_view_ptr->enable_blit_vr_views(true);
			vr_view_ptr->set_blit_vr_view_width(200);
		}
	}
	// more application specific init here

	return true;
}
~~~~~~~~~~~~~~~~~~~~~~~~
*/
class CGV_API vr_view_interactor : public stereo_view_interactor, public vr::vr_calibration_base
{
protected:
	using vec4 = cgv::vec4;
	using ivec4 = cgv::ivec4;
	using mat3 = cgv::mat3;
	using mat34 = cgv::mat34;
	using rgb = cgv::rgb;

	ivec4 cgv_viewport;
	void* fbo_handle;

	/**@name head tracking */
	//@{
	/// head orientation from tracker orientation
	mat3 head_tracker_orientation;
	/// head position from tracker location
	vec3 head_tracker_position;
	//@}

	/**@name calibation of tracker coordinate system
	
	the calibration affects the tracked poses as follows:

	mat3 rotation = cgv::math::rotate3<float>(tracking_rotation, vec3(0, 1, 0));
	mat3 orientation_world = rotation * orientation_raw;
	vec3 position_world = rotation * (position_raw - tracking_rotation_origin) + tracking_origin;

	or more brief:
	O = R*Q;
	p = R*(q-q0) + o;

	p = R*(q-q0) + o;
	o = p - R*(q - q0);

	to identify the current focus point f in world coordinates with the raw tracker position q and the rotation origin also with q we compute

	q0 = q;
	o  = p;

	Now rotation will happen around calibration point.
	*/
	//@{
	/// rotation angle around the y-axis
	float tracking_rotation;
	/// location in tracking coordinate system around which rotation is defined
	vec3 tracking_rotation_origin;
	/// origin of tracking coordinate system given in world coordinates
	vec3 tracking_origin;
	/// perform driver calibration
	void calibrate_driver();
	/// path to calibration file
	std::string calibration_file_path;
public:
	void set_tracking_rotation(float tr) {
		tracking_rotation = tr;
		calibrate_driver();
	}
	float get_tracking_rotation() {
		return tracking_rotation;
	}
	void set_tracking_origin(vec3 ori) {
		tracking_origin = ori;
		calibrate_driver();
	}
	vec3 get_tracking_origin() {
		return tracking_origin;
	}
	//@}
private:
	mat34 start_pose;
protected:
	/// whether the window shows a separate view onto the scene or the one of the current vr kit
	bool separate_view;
	/// whether to not render for kits
	bool dont_render_kits;
	/// whether to blit in the views of the vr kits
	bool blit_vr_views;
	// extent of blitting
	int blit_width;
	/// scale of aspect ratio used for blitting
	float blit_aspect_scale;
	/// selection of view of current hmd used in case of no separate view (1 ... left, 2 ... right, 3 ... both)
	int none_separate_view;
	int head_tracker;
	/// rendered_eye: 0...monitor,1...left eye,2...right eye
	int rendered_eye;
	vr::gl_vr_display* rendered_display_ptr;
	int rendered_display_index;

	cgv::gui::VREventTypeFlags event_flags;

	// debugging of vr events on console
	bool debug_vr_events;

	// visualization of kits and action zone
	VRkitVisType vis_type, hmd_vis_type, controller_vis_type, tracker_vis_type, base_vis_type;
	bool show_action_zone;
	rgb fence_color1, fence_color2;
	float fence_frequency;
	float fence_line_width;
	
	// current vr kit handle is selected by an integer cast into an enum whos names correspond to the vr kit names
	void* current_vr_handle;
	int current_vr_handle_index;
	std::string kit_enum_definition;
	// list of vr kit states
	std::vector<vr::vr_kit_state> kit_states;
	// list of to be initialized vr kits
	std::vector<void*> new_kits;
	// list of vr kits
	std::vector<void*> kits;
	// list of to be destructed vr kits
	std::vector<void*> old_kits;
	/// type of pose query according to vr::vr_kit::query_state function's 2nd parameter
	int pose_query;

	// render objects
	cgv::render::box_renderer br;
	cgv::render::surface_render_style brs;
	cgv::render::sphere_render_style srs;

	// helper members to allow change of mesh file names
	std::string hmd_mesh_file_name, controller_mesh_file_name, tracker_mesh_file_name, base_mesh_file_name;
	// for each mesh type a scale
	float mesh_scales[4];
	///
	vr::vr_kit* get_vr_kit_from_index(int i) const;
	//
	void configure_kits();
	///
	void on_status_change(void* handle, int controller_index, vr::VRStatus old_status, vr::VRStatus new_status);
	///
	virtual void on_device_change(void* handle, bool attach);
	/// helper to visualize pose with colored spheres
	void add_trackable_spheres(const float* pose, int i, std::vector<vec4>& spheres, std::vector<rgb>& sphere_colors);
public:
	///
	vr_view_interactor(const char* name);
	/// return the type name 
	std::string get_type_name() const;

	/**@name vr event processing*/
	//@{
	/// query the currently set event type flags
	cgv::gui::VREventTypeFlags get_event_type_flags() const;
	/// set the event type flags of to be emitted events
	void set_event_type_flags(cgv::gui::VREventTypeFlags flags);
	/// check whether vr events are printed to the console window
	bool vr_event_debugging_enabled() const { return debug_vr_events; }
	/// set whether vr events should be printed to the console window
	void enable_vr_event_debugging(bool enable = true);
	/// return a pointer to the state of the current vr kit
	const vr::vr_kit_state* get_current_vr_state() const;
	/// return a pointer to the current vr kit
	vr::vr_kit* get_current_vr_kit() const;
	// query vr state
	void query_vr_states();
	//@}

	/**@name vr viewing*/
	//@{
	//! query view direction of a vr kit
	/*! if parameter vr_kit_idx defaults to -1, the view direction of the current vr kit is returned
	    if there are not vr kits or the \c vr_kit_idx parameter is invalid the view direction of the 
		\c vr_view_interactor is returned*/
	dvec3 get_view_dir_of_kit(int vr_kit_idx = -1) const;
	//! query view up direction of a vr kit
	/*! if parameter vr_kit_idx defaults to -1, the view up direction of the current vr kit is returned
		if there are not vr kits or the \c vr_kit_idx parameter is invalid the view up direction of the
		\c vr_view_interactor is returned*/
	dvec3 get_view_up_dir_of_kit(int vr_kit_idx = -1) const;
	//! query the eye position of a vr kit.
	/*! parameter \c eye is one of
	    -1 .. left eye
		 0 .. cyclopic eye
		 1 .. right eye
		if parameter vr_kit_idx defaults to -1, the eye position of the current vr kit is returned
		if there are not vr kits or the \c vr_kit_idx parameter is invalid the eye position of the
		\c vr_view_interactor is returned*/
	dvec3 get_eye_of_kit(int eye = 0, int vr_kit_idx = -1) const;
	//@}
	
	/**@name vr rendering*/
	//@{
	/// check whether separate view is rendered
	bool seperate_view_drawn() const { return separate_view; }
	/// set whether to draw separate view
	void draw_separate_view(bool do_draw);
	/// check whether vr kits are drawn
	bool vr_kits_drawn() const { return vis_type != VVT_NONE; }
	/// set whether to draw vr kits
	void draw_vr_kits(bool do_draw);
	/// set whether to draw controllers
	void draw_vr_controllers(bool do_draw);
	/// check whether action zone is drawn 
	bool action_zone_drawn() const { return show_action_zone; }
	/// whether to draw action zone
	void draw_action_zone(bool do_draw);
	/// check whether vr views are blitted
	bool blit_vr_views_enabled() const { return blit_vr_views; }
	/// enable vr view blitting
	void enable_blit_vr_views(bool enable);
	/// return width of vr view blitting
	int get_blit_vr_view_width() const { return blit_width; }
	/// set the width with which vr views are blit
	void set_blit_vr_view_width(int width);
	//@}

	/**@name vr render process*/
	//@{
	/// return the currently rendered eye
	int get_rendered_eye() const { return rendered_eye; }
	/// return the vr kit currently rendered
	/// return a pointer to the current vr kit
	vr::gl_vr_display* get_rendered_display() const { return rendered_display_ptr; }
	/// return pointer to rendered vr kit or nullptr if birds eye view is rendered
	vr::vr_kit* get_rendered_vr_kit() const { return static_cast<vr::vr_kit*>(rendered_display_ptr); }
	/// return index of rendered vr kit or -1 if birds eye view is rendered
	int get_rendered_vr_kit_index() const { return rendered_display_index; }
	//@}
	/// 
	void on_set(void* member_ptr);
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	///
	bool init(cgv::render::context& ctx);
	/// 
	void clear(cgv::render::context& ctx);
	/// overload and implement this method to handle events
	bool handle_vr_events(cgv::gui::event& e);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	// render the views for the kits in nested render passes
	void render_vr_kits(cgv::render::context& ctx);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context&);
	/// draw the vr kits in the current view
	void draw_vr_kits(cgv::render::context& ctx);
	/// draw the action zone of the current vr kit
	void draw_action_zone(cgv::render::context& ctx);
	/// draw all
	void draw(cgv::render::context&);
	/// this method is called in one pass over all drawables after drawing
	void finish_frame(cgv::render::context&);
	/// this method is called in one pass over all drawables after finish frame
	void after_finish(cgv::render::context& ctx);
	///
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// you must overload this for gui creation
	void create_gui();
};
///@}

#include <cgv/config/lib_end.h>
