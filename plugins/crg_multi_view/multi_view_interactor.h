#pragma once

#include <cgv/base/base.h>
#include <vr/vr_kit.h>
#include <cg_vr/vr_server.h>
#include <vr/vr_driver.h>
#include <vr/vr_kit.h>
#include <cgv_gl/box_renderer.h>
#include <cgv_gl/sphere_renderer.h>
#include <cgv_gl/rectangle_renderer.h>
#include <cgv_gl/gl/mesh_render_info.h>
#include <vr_view_interactor.h>

#include "lib_begin.h"

/** for each tracker of a user, we store additional information*/
struct head_tracking_info
{
	/// pointer to vr_kit to which the tracker is attached
	void* kit_handle;
	/// controller index of tracker
	int controller_index;
	/// position of left (index 0) and right (index 1) eye in local coordinate system of tracker
	cgv::vec3 local_eye_position[2];
};

enum MultiViewMode
{
	MVM_DEBUG = 0,
	MVM_TWO_PLAYERS_PASSIVE_ANAGLYPH = 1,
	MVM_TWO_PLAYERS_PASSIVE_ACTIVE = 2,
	MVM_FOUR_PLAYERS = 3,
	MVM_LAST = MVM_FOUR_PLAYERS
};

class CGV_API multi_view_interactor : public vr_view_interactor 
{
protected:
	using vec2 = cgv::vec2;
	using vec3 = cgv::vec3;
	using mat4 = cgv::mat4;
	using quat = cgv::quat;

	void* last_kit_handle;
	///
	MultiViewMode multi_view_mode;
	
	/**@name player information*/
	//@{
	/// store a list of trackers used to track head positions of players
	std::vector<head_tracking_info> head_trackers;
	/// list of displays used for offline rendering
	std::vector<vr::gl_vr_display*> displays;
	/// list of uninitialized displays 
	std::vector<vr::gl_vr_display*> new_displays;
	/// return position of eye (0 ..left | 1 ..right) in tracker coordinates
	inline vec3 get_eye_position_tracker(unsigned user_index, int eye) const { return head_trackers[user_index].local_eye_position[eye]; }
	/// given tracker pose, return position of eye (0 ..left | 1 ..right) in world coordinates
	inline vec3 get_eye_position_world(unsigned user_index, int eye, const mat34& tracker_pose) const { return tracker_pose * vec4(get_eye_position_tracker(user_index, eye), 1.0f); }
	/// given tracker pose construct 4x4 matrix in column major format for perspective transformation from eye (0..left, 1..right)
	void put_projection_matrix(unsigned user_index, int eye, float z_near, float z_far, float* projection_matrix, const float* tracker_pose) const;
	/// given tracker pose construct lookat matrix for a given eye (0 ... left, 1 ... right)
	void put_world_to_eye_transform(unsigned user_index, int eye, const float* tracker_pose, float* modelview_matrix, vec3* eye_world_ptr = 0, vec3* view_dir_ptr = 0) const;
	//@}

	/**@name screen information*/
	//@{
	/// store screen coordinate and position in a way that it can be cast into a pose matrix
	mat34 screen_pose;
	/// x- and y-pixel sizes in meters
	vec2 pixel_size;
	/// pixel extent of screen
	unsigned screen_width, screen_height;
	/// read access to screen orientation
	inline const mat3& get_screen_orientation() const { return reinterpret_cast<const mat3&>(screen_pose); }
	/// write access to screen orientation
	inline void set_screen_orientation(const quat& q) { q.put_matrix(reinterpret_cast<mat3&>(screen_pose)); screen_orientation = q; }
	/// access to screen center location
	inline const vec3& get_screen_center() const { return screen_pose.col(3); }
	/// transform to coordinate system of screen with [0,0,0] in center and corners [+-aspect,+-1,0]; z is signed distance to screen in world unites (typically meters) 
	vec3 transform_world_to_screen(const vec3& p) const;
	/// transform from coordinate system of screen with [0,0,0] in center and corners [+-aspect,+-1,0]; z is signed distance to screen in world unites (typically meters) 
	vec3 transform_screen_to_world(const vec3& p) const;
	//@}
	/// whether to render the screen inside of the scene
	bool show_screen;
	/// this member variable is only used for the user interface to allow changes to the orientation of the screen
	quat screen_orientation;
	/// 
	float pixel_scale;
	/// whether to show the eyes of the users
	bool show_eyes;
	/// index user of whom to show rendering on the virtual screen
	int debug_display_index;
	/// index of eye rendered for debugging on virtual screen
	int debug_eye;
	///
	bool show_probe;
	/// debug probe
	vec3 debug_probe;
	/// 
	int add_controller_as_player;
	///
	cgv::render::rectangle_render_style rrs;
	///
	const vr::vr_trackable_state* get_trackable_state(int display_index) const;
	///
	void on_device_change(void* handle, bool attach);
public:
	///
	multi_view_interactor(const char* name);
	/// return the type name 
	std::string get_type_name() const;
	///
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// attach player to controller/tracker ci of last attached vr kit
	void add_player(int ci);
	/// general callback
	void on_set(void* member_ptr);
	/// 
	bool init(cgv::render::context& ctx);
	///
	bool handle(cgv::gui::event& e);
	/// 
	void clear(cgv::render::context& ctx);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context& ctx);
	/// 
	void draw(cgv::render::context& ctx);
	/// this method is called at the end of a render pass
	void after_finish(cgv::render::context& ctx);
	///
	void create_gui();
};
///@}

#include <cgv/config/lib_end.h>
