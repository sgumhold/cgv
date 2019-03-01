#pragma once

#include <cgv/base/base.h>
#include <vr/vr_kit.h>
#include <cg_vr/vr_server.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <stereo_view_interactor.h>

#include "lib_begin.h"

///@ingroup VR
///@{

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
class CGV_API vr_view_interactor : 
	public stereo_view_interactor	
{
protected:
	/// whether the window shows a separate view onto the scene or the one of the current vr kit
	bool separate_view;
	/// whether to blit in the views of the vr kits
	bool blit_vr_views;
	// extent of blitting
	int blit_width;

	int rendered_eye;
	vr::vr_kit* rendered_kit_ptr;
	int rendered_kit_index;
	cgv::gui::VREventTypeFlags event_flags;
	static dmat4 hmat_from_pose(float pose_matrix[12]);

	// debugging of vr events on console
	bool debug_vr_events;

	// visualization of kits and action zone
	bool show_vr_kits;
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

	// render objects
	cgv::render::box_renderer br;
	cgv::render::surface_render_style brs;
	cgv::render::sphere_render_style srs;

	//
	void configure_kits();
	///
	void on_status_change(void* device_handle, int controller_index, vr::VRStatus old_status, vr::VRStatus new_status);
	///
	void on_device_change(void* device_handle, bool attach);
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
	//@}

	/**@name vr rendering*/
	//@{
	/// check whether separate view is rendered
	bool seperate_view_drawn() const { return separate_view; }
	/// set whether to draw separate view
	void draw_separate_view(bool do_draw);
	/// check whether vr kits are drawn
	bool vr_kits_drawn() const { return show_vr_kits; }
	/// set whether to draw vr kits
	void draw_vr_kits(bool do_draw);
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
	/// 
	void on_set(void* member_ptr);
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	///
	bool init(cgv::render::context& ctx);
	/// 
	void destruct(cgv::render::context& ctx);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context&);
	/// 
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
