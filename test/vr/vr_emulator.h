#pragma once

#include <cgv/base/base.h>
#include <cgv/render/drawable.h>
#include <cgv/math/quaternion.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <vr/vr_kit.h>
#include <vr/vr_driver.h>

#include "lib_begin.h"

///@ingroup VR
///@{

/**@file 
provides vr_emulator and vr_emulated_kit classes.
*/
class CGV_API vr_emulator;

enum TrackerAttachment 
{
	TA_WORLD,
	TA_HEAD,
	TA_LEFT_HAND,
	TA_RIGHT_HAND
};

class CGV_API vr_emulated_kit : public vr::vr_kit
{
public:
	using vec3 = cgv::vec3;
	using vec4 = cgv::vec4;
	using mat3 = cgv::mat3;
	using mat4 = cgv::mat4;
	using mat34 = cgv::mat34;
protected:
	friend class vr_emulator;
	vr::vr_kit_state state;
	typedef cgv::math::quaternion<float> quat;
	float body_direction;
	float body_height;
	float hip_parameter;
	float yaw_parameter;
	float fovy;
	vec3 body_position;

	vec3 hand_position[2];
	quat hand_orientation[2];

	vec3 tracker_positions[4];
	quat tracker_orientations[4];
	bool tracker_enabled[4];
	TrackerAttachment tracker_attachments[4];

	/// helper functions to construct matrices
	mat34 construct_pos_matrix(const quat& orientation, const vec3& position);
	mat4 construct_homogeneous_matrix(const quat& orientation, const vec3& position);
	void set_pose_matrix(const mat4& H, float* pose) const;
	void compute_state_poses();
	bool query_state_impl(vr::vr_kit_state& state, int pose_query = 2);
public:
	vr_emulated_kit(float _body_direction, const vec3& _body_position, float _body_height, unsigned _width, unsigned _height, vr::vr_driver* _driver, void* _handle, const std::string& _name, int _nr_trackers);
	vec3 get_body_direction() const;
	const std::vector<std::pair<int, int> >& get_controller_throttles_and_sticks(int controller_index) const;
	const std::vector<std::pair<float, float> >& get_controller_throttles_and_sticks_deadzone_and_precision(int controller_index) const;
	bool set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength);
	void put_eye_to_head_matrix(int eye, float* pose_matrix) const;
	void put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix, const float* hmd_pose) const;
	void submit_frame();
};

enum InteractionMode {
	IM_BODY,
	IM_LEFT_HAND,
	IM_RIGHT_HAND,
	IM_TRACKER_1,
	IM_TRACKER_2,
	IM_TRACKER_3,
	IM_TRACKER_4,
	IM_BASE_1,
	IM_BASE_2,
	IM_BASE_3,
	IM_BASE_4
};

class CGV_API vr_emulator : 
	public cgv::base::node, public cgv::render::drawable,
	public vr::vr_driver, public cgv::gui::provider, public cgv::gui::event_handler
{
public:
	using quat = cgv::quat;
	using vec3 = cgv::vec3;
	using vec4 = cgv::vec4;
	using mat3 = cgv::mat3;
	using mat34 = cgv::mat34;

	std::vector<vr_emulated_kit*> kits;
	static bool gamepad_connected;
	unsigned screen_width, screen_height;
	unsigned counter;
	int nr_trackers = 2;
	vec3 body_position;
	float body_direction;
	float body_height;
	InteractionMode interaction_mode;
	quat hand_orientation[2];

	// coordinate transform used to emulate a displacement between tracking and world system
	quat coordinate_rotation;
	vec3 coordinate_displacement;
	// information used to define reference states
	std::vector<quat> base_orientations;
	std::vector<vec3> base_positions;
	std::vector<std::string> base_serials;
	/// update a single renference state or all from base_orientations, base_positions and base_serials
	void update_reference_states(int i = -1);

protected:
	mutable std::map<std::string, vr::vr_trackable_state> transformed_reference_states;
	
	// emulation
	bool installed;
	float body_speed;

	bool left_ctrl, right_ctrl, up_ctrl, down_ctrl;
	bool home_ctrl, end_ctrl, pgup_ctrl, pgdn_ctrl;
	bool is_alt;

	void create_tracker_gui(vr_emulated_kit* kit, int i);
	void create_trackable_gui(const std::string& name, vr::vr_trackable_state& ts);
	void create_controller_gui(int i, vr::vr_controller_state& cs);

	int current_kit_index;

	void add_new_kit();
	void timer_event(double t, double dt);
public:
	///
	vr_emulator();
	///
	void on_set(void* member_ptr);
	/// return name of driver
	std::string get_driver_name() const;
	/// return whether driver is installed
	bool is_installed() const;
	/// scan all connected vr kits and return a vector with their ids
	std::vector<void*> scan_vr_kits();
	/// scan all connected vr kits and return a vector with their ids
	vr::vr_kit* replace_by_index(int& index, vr::vr_kit* new_kit_ptr);
	/// scan all connected vr kits and return a vector with their ids
	bool replace_by_pointer(vr::vr_kit* old_kit_ptr, vr::vr_kit* new_kit_ptr);
	/// put a 3d up direction into passed array
	void put_up_direction(float* up_dir) const;
	/// return the floor level relativ to the world origin
	float get_floor_level() const;
	/// return height of action zone in meters
	float get_action_zone_height() const;
	/// return a vector of floor points defining the action zone boundary as a closed polygon
	void put_action_zone_bounary(std::vector<float>& boundary) const;
	///
	bool check_for_button_toggle(cgv::gui::key_event& ke, int controller_index, vr::VRButtonStateFlags button, float touch_x = 0.0f, float touch_y = 0.0f);
	///
	bool handle_ctrl_key(cgv::gui::key_event& ke, bool& fst_ctrl, bool* snd_ctrl_ptr = 0);
	/// overload and implement this method to handle events
	bool handle(cgv::gui::event& e);
	/// overload to stream help information to the given output stream
	void stream_help(std::ostream& os);
	/// return the type name 
	std::string get_type_name() const;
	/// overload to show the content of this object
	void stream_stats(std::ostream&);
	///
	bool init(cgv::render::context& ctx);
	/// this method is called in one pass over all drawables before the draw method
	void init_frame(cgv::render::context&);
	/// 
	void draw(cgv::render::context&);
	/// this method is called in one pass over all drawables after drawing
	void finish_frame(cgv::render::context&);
	///
	bool self_reflect(cgv::reflect::reflection_handler& srh);
	/// you must overload this for gui creation
	void create_gui();
};
///@}

#include <cgv/config/lib_end.h>
