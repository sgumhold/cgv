#pragma once

#include <cgv/base/base.h>
#include <vr/vr_event.h>
#include <vr/vr_kit.h>
#include <cg_vr/vr_server.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <plugins/crg_stereo_view/stereo_view_interactor.h>

#include "lib_begin.h"

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
	cgv::render::sphere_renderer sr;
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

	/// query the currently set event type flags
	cgv::gui::VREventTypeFlags get_event_type_flags() const;
	/// set the event type flags of to be emitted events
	void set_event_type_flags(cgv::gui::VREventTypeFlags flags);
	/// return a pointer to the state of the current vr kit
	const vr::vr_kit_state* get_current_vr_state() const;
	/// 
	void on_set(void* member_ptr);
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	///
	bool init(cgv::render::context& ctx);
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

#include <cgv/config/lib_end.h>
