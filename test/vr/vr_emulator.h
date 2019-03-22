#pragma once

#include <cgv/base/base.h>
#include <cgv/render/drawable.h>
#include <cgv/math/quaternion.h>
#include <cgv/gui/provider.h>
#include <cgv/gui/event_handler.h>
#include <cgv/gui/key_event.h>
#include <vr/gl_vr_display.h>
#include <vr/vr_driver.h>

#include "lib_begin.h"

///@ingroup VR
///@{

/**@file 
provides vr_emulator and vr_emulated_kit classes.
*/
class CGV_API vr_emulator;

class CGV_API vr_emulated_kit : public vr::gl_vr_display, public cgv::render::render_types
{
public:
	typedef cgv::math::fmat<float, 3, 4> mat3x4;
protected:
	friend class vr_emulator;
	vr::vr_kit_state state;
	typedef cgv::math::quaternion<float> quat;
	float body_direction;
	float body_height;
	float hip_parameter;
	float gear_parameter;
	float fovy;
	vec3 body_position;
	vec3 hand_position[2];

	/// helper functions to construct matrices
	mat3x4 construct_pos_matrix(const quat& orientation, const vec3& position);
	mat4 construct_homogeneous_matrix(const quat& orientation, const vec3& position);
	void set_pose_matrix(const mat4& H, float* pose);
	void compute_state_poses();
public:
	vr_emulated_kit(float _body_direction, const vec3& _body_position, float _body_height, unsigned _width, unsigned _height, vr::vr_driver* _driver, void* _handle, const std::string& _name, bool _ffb_support, bool _wireless);
	vec3 get_body_direction() const;
	const std::vector<std::pair<int, int> >& get_controller_throttles_and_sticks(int controller_index) const;
	const std::vector<std::pair<float, float> >& get_controller_throttles_and_sticks_deadzone_and_precision(int controller_index) const;
	bool query_state(vr::vr_kit_state& state, int pose_query = 2);
	bool set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength);
	void put_eye_to_head_matrix(int eye, float* pose_matrix);
	void put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix);
	void submit_frame();
};

class CGV_API vr_emulator : 
	public cgv::base::node, public cgv::render::drawable,
	public vr::vr_driver, public cgv::gui::provider, public cgv::gui::event_handler
{
public:
	typedef cgv::math::quaternion<float> quat;
	std::vector<vr_emulated_kit*> kits;
	unsigned screen_width, screen_height;
	bool ffb_support, wireless;
	unsigned counter;
	vec3 body_position;
	float body_direction;
	float body_height;
protected:
	// emulation
	bool installed;
	float body_speed;

	bool left_ctrl, right_ctrl, up_ctrl, down_ctrl;
	bool home_ctrl, end_ctrl, pgup_ctrl, pgdn_ctrl;
	int current_kit_ctrl;
	void create_trackable_gui(const std::string& name, vr::vr_trackable_state& ts);
	void create_controller_gui(int i, vr::vr_controller_state& cs);

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
	/// put a 3d up direction into passed array
	void put_up_direction(float* up_dir) const;
	/// return the floor level relativ to the world origin
	float get_floor_level() const;
	/// return height of action zone in meters
	float get_action_zone_height() const;
	/// return a vector of floor points defining the action zone boundary as a closed polygon
	void put_action_zone_bounary(std::vector<float>& boundary) const;
	///
	bool check_for_button_toggle(cgv::gui::key_event& ke, int controller_index, vr::VRButtonStateFlags button);
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

#include <cgv/config/lib_end.h>
