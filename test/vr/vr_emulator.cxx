#include "vr_emulator.h"
#include <cgv/math/ftransform.h>
#include <cgv/math/pose.h>
#include <cgv/utils/scan.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/trigger.h>
#include <cgv_reflect_types/math/fvec.h>
#include <cgv_reflect_types/math/quaternion.h>
#include <cg_vr/vr_server.h>
#include <cgv/utils/convert_string.h>
#include <cg_gamepad/gamepad_server.h>

const float Body_height = 1740.0f;
const float Eye_height = 1630.0f;
const float Chin_height = 1530.0f;
const float Shoulder_height = 1425.0f;
const float Shoulder_breadth = 485.0f;
const float Arm_span = 1790.0f;
const float Arm_length = 790.0f;
const float Hip_width = 360.0f;
const float Hip_height = 935.0f;
const float Elbow_height = 1090.0f;
const float Hand_height = 755.0f;
const float Reach_Upwards = 2060.0f;
const float Pupillary_distance = 63.0f;

vr_emulated_kit::mat34 vr_emulated_kit::construct_pos_matrix(const quat& orientation, const vec3& position)
{
	mat3 R;
	orientation.put_matrix(R);
	mat34 P;
	P.set_col(0, R.col(0));
	P.set_col(1, R.col(1));
	P.set_col(2, R.col(2));
	P.set_col(3, position);
	return P;
}

vr_emulated_kit::mat4 vr_emulated_kit::construct_homogeneous_matrix(const quat& orientation, const vec3& position)
{
	mat3 R;
	orientation.put_matrix(R);
	mat4 H;
	H.set_col(0, vec4(R(0, 0), R(1, 0), R(2, 0), 0));
	H.set_col(1, vec4(R(0, 1), R(1, 1), R(2, 1), 0));
	H.set_col(2, vec4(R(0, 2), R(1, 2), R(2, 2), 0));
	H.set_col(3, vec4(position(0), position(1), position(2), 1.0f));
	return H;
}

vr_emulated_kit::vec3 vr_emulated_kit::get_body_direction() const
{
	vec3 up_dir;
	vec3 x_dir, z_dir;
	driver->put_x_direction(&x_dir(0));
	driver->put_up_direction(&up_dir(0));
	z_dir = cross(x_dir, up_dir);
	return -sin(body_direction)*x_dir + cos(body_direction)*z_dir;
}

void vr_emulated_kit::compute_state_poses()
{
	float scale = body_height / Body_height;
	vec3 up_dir;
	vec3 x_dir, z_dir;
	driver->put_x_direction(&x_dir(0));
	driver->put_up_direction(&up_dir(0));
	z_dir = cross(x_dir, up_dir);
	mat4 T_body;
	T_body.set_col(0, vec4(cos(body_direction)*x_dir + sin(body_direction)*z_dir, 0));
	T_body.set_col(1, vec4(up_dir, 0));
	T_body.set_col(2, vec4(-sin(body_direction)*x_dir + cos(body_direction)*z_dir,0));
	T_body.set_col(3, vec4(body_position, 1));
	mat4 T_hip = 
		cgv::math::translate4<float>(vec3(0,scale*Hip_height,0))*
		cgv::math::rotate4<float>(-60*hip_parameter, vec3(1, 0, 0));
	mat4 T_head =
		cgv::math::translate4<float>(vec3(0, scale*(Chin_height - Hip_height), 0))*
		cgv::math::rotate4<float>(-90*yaw_parameter, vec3(0, 1, 0));
	mat4 R;
	hand_orientation[0].put_homogeneous_matrix(R);
	mat4 T_left =
		cgv::math::translate4<float>(
			scale*vec3((-Shoulder_breadth + Arm_length * hand_position[0](0)),
				Shoulder_height - Hip_height + Arm_length * hand_position[0](1),
				Arm_length*hand_position[0](2)))*R;
	hand_orientation[1].put_homogeneous_matrix(R);
	mat4 T_right =
		cgv::math::translate4<float>(
			scale*vec3(+(Shoulder_breadth + Arm_length * hand_position[1](0)),
				Shoulder_height - Hip_height + Arm_length * hand_position[1](1),
				Arm_length*hand_position[1](2)))*R;

	set_pose_matrix(T_body*T_hip*T_head, state.hmd.pose);
	set_pose_matrix(T_body*T_hip*T_left, state.controller[0].pose);
	set_pose_matrix(T_body*T_hip*T_right, state.controller[1].pose);
	static unsigned ts = 1;
	++ts;
	state.controller[0].time_stamp = ts;
	state.controller[1].time_stamp = ts;
	for (int i = 0; i < 4; ++i) {
		if (!tracker_enabled[i]) {
			state.controller[2 + i].status = vr::VRS_DETACHED;
			continue;
		}
		mat4 T = construct_homogeneous_matrix(tracker_orientations[i], tracker_positions[i]);
		switch (tracker_attachments[i]) {
		case TA_HEAD: T = T_body * T_hip*T_head*T; break;
		case TA_LEFT_HAND: T = T_body * T_hip*T_left*T; break;
		case TA_RIGHT_HAND: T = T_body * T_hip*T_right*T; break;
		}
		set_pose_matrix(T, state.controller[2 + i].pose);
		state.controller[2 + i].status = vr::VRS_TRACKED;
		state.controller[2 + i].time_stamp = ts;
	}
}

vr_emulated_kit::vr_emulated_kit(float _body_direction, const vec3& _body_position, float _body_height, unsigned _width, unsigned _height, vr::vr_driver* _driver, void* _handle, const std::string& _name, int _nr_trackers)
	: vr_kit(_driver, _handle, _name, _width, _height)
{
	body_position = _body_position;
	body_direction=_body_direction;
	body_height = _body_height;
	hip_parameter= 0;
	yaw_parameter = 0;
	fovy = 90;
	hand_position[0] = vec3(0, -0.5f, -0.2f);
	hand_position[1] = vec3(0, -0.5f, -0.2f);
	hand_orientation[0] = quat(1, 0, 0, 0);
	hand_orientation[1] = quat(1, 0, 0, 0);
	state.hmd.status = vr::VRS_TRACKED;
	state.controller[0].status = vr::VRS_TRACKED;
	state.controller[1].status = vr::VRS_TRACKED;

	for (int i=0; i<4; ++i)
		tracker_enabled[i] = i < _nr_trackers;

	tracker_positions[0] = vec3(0.2f, 1.2f, 0.0f);
	tracker_positions[1] = vec3(-0.2f, 1.2f, 0.0f);
	tracker_positions[2] = vec3(-0.6f, 1.2f, 0.0f);
	tracker_positions[3] = vec3(0.6f, 1.2f, 0.0f);
	tracker_orientations[0] = tracker_orientations[1] = tracker_orientations[2] = tracker_orientations[3] = quat(0.71f,-0.71f,0,0);
	tracker_attachments[0] = tracker_attachments[1] = tracker_attachments[2] = tracker_attachments[3] = TA_WORLD;
	info.force_feedback_support = true;
	info.hmd.device_class = 1234;
	info.hmd.device_type = "vr kit";
	info.hmd.has_proximity_sensor = false;
	info.hmd.head_to_eye_distance = 0.1f;
	info.hmd.ipd = 0.06f;
	info.hmd.model_number = 1;
	info.hmd.number_cameras = 0;
	info.hmd.serial_number = cgv::utils::to_string(_handle)+"H";
	info.hmd.fps = 60;
	for (int ci = 0; ci < 6; ++ci) {
		if (ci < 2) {
			info.controller[ci].type = vr::VRC_CONTROLLER;
			info.controller[ci].role = ci == 0 ? vr::VRC_LEFT_HAND : vr::VRC_RIGHT_HAND;
			info.controller[ci].nr_axes = 3;
			info.controller[ci].nr_inputs = 2;
			info.controller[ci].input_type[0] = vr::VRI_PAD;
			info.controller[ci].input_type[1] = vr::VRI_TRIGGER;
			info.controller[ci].axis_type[0] = vr::VRA_PAD_X;
			info.controller[ci].axis_type[1] = vr::VRA_PAD_Y;
			info.controller[ci].axis_type[2] = vr::VRA_TRIGGER;
			info.controller[ci].supported_buttons = vr::VRButtonStateFlags(vr::VRF_MENU | vr::VRF_GRIP | vr::VRF_SYSTEM | vr::VRF_A | vr::VRF_INPUT0_TOUCH | vr::VRF_INPUT0);
		}
		else
			info.controller[ci].type = vr::VRC_TRACKER;

		info.controller[ci].model_number = ci < 2 ? 2 : 6;
		info.controller[ci].is_wireless = true;
		info.controller[ci].device_class = 444;
		info.controller[ci].device_type = ci < 2 ? "emul_controller" : "emul_tracker";
		info.controller[ci].serial_number = cgv::utils::to_string(_handle) + "C" + cgv::utils::to_string(ci);
	}
	compute_state_poses();
}

const std::vector<std::pair<int, int> >& vr_emulated_kit::get_controller_throttles_and_sticks(int controller_index) const
{
	static std::vector<std::pair<int, int> > throttles_and_sticks;
	if (throttles_and_sticks.empty()) {
		// add stick
		throttles_and_sticks.push_back(std::pair<int, int>(0, 1));
		// add trigger throttle
		throttles_and_sticks.push_back(std::pair<int, int>(2, -1));
	}
	return throttles_and_sticks;
}

const std::vector<std::pair<float, float> >& vr_emulated_kit::get_controller_throttles_and_sticks_deadzone_and_precision(int controller_index) const
{
	static std::vector<std::pair<float, float> > deadzone_and_precision;
	if (deadzone_and_precision.empty()) {
		deadzone_and_precision.push_back(std::pair<float, float>(0.1f, 0.01f));
		deadzone_and_precision.push_back(std::pair<float, float>(0.0f, 0.2f));
	}
	return deadzone_and_precision;
}


void vr_emulated_kit::set_pose_matrix(const mat4& H, float* pose) const
{
	pose[0]  = H(0, 0);
	pose[1]  = H(1, 0);
	pose[2]  = H(2, 0);
	pose[3]  = H(0, 1);
	pose[4]  = H(1, 1);
	pose[5]  = H(2, 1);
	pose[6]  = H(0, 2);
	pose[7]  = H(1, 2);
	pose[8]  = H(2, 2);
	pose[9]  = H(0, 3);
	pose[10] = H(1, 3);
	pose[11] = H(2, 3);
}

bool vr_emulated_kit::query_state_impl(vr::vr_kit_state& state, int pose_query)
{
	compute_state_poses();
	state = this->state;
	const vr_emulator* vr_em_ptr = dynamic_cast<const vr_emulator*>(get_driver());
	if (vr_em_ptr) {
		// transform state with coordinate transformation
		mat34 coordinate_transform;
		vr_em_ptr->coordinate_rotation.put_matrix(reinterpret_cast<mat3&>(coordinate_transform));
		reinterpret_cast<vec3&>(coordinate_transform(0, 3)) = vr_em_ptr->coordinate_displacement;
		cgv::math::pose_transform(coordinate_transform, reinterpret_cast<mat34&>(state.hmd.pose[0]));
		for (int ci = 0; ci < 6; ++ci)
			cgv::math::pose_transform(coordinate_transform, reinterpret_cast<mat34&>(state.controller[ci].pose[0]));
	}
	return true;
}

bool vr_emulated_kit::set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength)
{
	state.controller[controller_index].vibration[0] = low_frequency_strength;
	state.controller[controller_index].vibration[1] = high_frequency_strength;
	return info.force_feedback_support;
}

void vr_emulated_kit::put_eye_to_head_matrix(int eye, float* pose_matrix) const
{
	float scale = body_height / Body_height;
	set_pose_matrix(
		cgv::math::translate4<float>(
			scale*vec3(float(eye - 0.5f)*Pupillary_distance, Eye_height - Chin_height, -Pupillary_distance)
		), pose_matrix
	);
}

void vr_emulated_kit::put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix, const float*) const
{
	reinterpret_cast<mat4&>(*projection_matrix) = 
		cgv::math::perspective4<float>(fovy, float(width)/height, z_near, z_far);
}

void vr_emulated_kit::submit_frame()
{

}

///
vr_emulator::vr_emulator() : cgv::base::node("vr_emulator")
{
	current_kit_index = -1;
	interaction_mode = IM_BODY;

	left_ctrl = right_ctrl = up_ctrl = down_ctrl = false;
	home_ctrl = end_ctrl = pgup_ctrl = pgdn_ctrl = false;
	installed = true;
	body_speed = 1.0f;
	body_position = vec3(0, 0, 1);
	body_height = 1.75f;
	body_direction = 0;
	screen_width = 640;
	screen_height = 480;
	counter = 0;

	coordinate_rotation = quat(1,0,0,0);
	coordinate_displacement = vec3(0.0f);

	ref_tracking_reference_state("vr_emulator_base_01").status = vr::VRS_TRACKED;
	mat3& ref_ori_1 = reinterpret_cast<mat3&>(*ref_tracking_reference_state("vr_emulator_base_01").pose);

	base_orientations.push_back(quat(cgv::math::rotate3<float>(vec3(-20.0f, 45.0f, 0))));
	base_positions.push_back(vec3(1.0f, 2.0f, 1.0f));
	base_serials.push_back("vr_emulator_base_01");

	base_orientations.push_back(quat(cgv::math::rotate3<float>(vec3(-20.0f, -45.0f, 0))));
	base_positions.push_back(vec3(-1.0f, 2.0f, 1.0f));
	base_serials.push_back("vr_emulator_base_02");

	update_reference_states();

	connect(cgv::gui::get_animation_trigger().shoot, this, &vr_emulator::timer_event);
}

/// update a single renference state or all from base_orientations, base_positions and base_serials
void vr_emulator::update_reference_states(int i)
{
	int ib = i, ie = i + 1;
	if (i == -1) {
		ib = 0;
		ie = (int)base_serials.size();
	}
	mat34 coordinate_transform = pose_construct(coordinate_rotation, coordinate_displacement);
	for (int k = ib; k < ie; ++k) {
		auto& pose = reinterpret_cast<mat34&>(ref_tracking_reference_state(base_serials[k]).pose[0]);
		ref_tracking_reference_state(base_serials[k]).status = vr::VRS_TRACKED;
		base_orientations[k].put_matrix(pose_orientation(pose));
		pose_position(pose) = base_positions[k];
		pose_transform(coordinate_transform, pose);
	}
}

void vr_emulator::timer_event(double t, double dt)
{
	if (current_kit_index >= 0 && current_kit_index < (int)kits.size()) {
		switch (interaction_mode)
		{
		case IM_BODY:
			if (left_ctrl || right_ctrl) {
				if (is_alt)
					kits[current_kit_index]->body_position -= (float)(left_ctrl ? -dt : dt) * cross(kits[current_kit_index]->get_body_direction(), vec3(0, 1, 0));
				else 
					kits[current_kit_index]->body_direction += 3 * (float)(left_ctrl ? -dt : dt);
				update_all_members();
				post_redraw();
			}
			if (up_ctrl || down_ctrl) {
				kits[current_kit_index]->body_position -= (float)(down_ctrl ? -dt : dt) * kits[current_kit_index]->get_body_direction();
				update_all_members();
				post_redraw();
			}
			if (home_ctrl || end_ctrl) {
				kits[current_kit_index]->yaw_parameter += (float)(home_ctrl ? -dt : dt);
				if (kits[current_kit_index]->yaw_parameter < -1)
					kits[current_kit_index]->yaw_parameter = -1;
				else if (kits[current_kit_index]->yaw_parameter > 1)
					kits[current_kit_index]->yaw_parameter = 1;
				update_all_members();
				post_redraw();
			}
			if (pgup_ctrl || pgdn_ctrl) {
				kits[current_kit_index]->hip_parameter += (float)(pgup_ctrl ? -dt : dt);
				if (kits[current_kit_index]->hip_parameter < -1)
					kits[current_kit_index]->hip_parameter = -1;
				else if (kits[current_kit_index]->hip_parameter > 1)
					kits[current_kit_index]->hip_parameter = 1;
				update_all_members();
				post_redraw();
			}
			break;
		case IM_LEFT_HAND:
		case IM_RIGHT_HAND:
			if (home_ctrl || end_ctrl) {
				float& value = kits[current_kit_index]->state.controller[interaction_mode - IM_LEFT_HAND].axes[2];
				value += 1.0f * float(home_ctrl ? -dt : dt);
				value = std::max(std::min(1.0f, value), 0.0f);
				update_member(&value);
				post_redraw();
				break;
			}
		case IM_TRACKER_1:
		case IM_TRACKER_2:
		case IM_TRACKER_3:
		case IM_TRACKER_4:
		case IM_BASE_1:
		case IM_BASE_2:
		case IM_BASE_3:
		case IM_BASE_4:
		{
			quat* orientation_ptr = 0;
			vec3* position_ptr = 0;
			if (interaction_mode < IM_TRACKER_1) {
				orientation_ptr = &kits[current_kit_index]->hand_orientation[interaction_mode - IM_LEFT_HAND];
				position_ptr = &kits[current_kit_index]->hand_position[interaction_mode - IM_LEFT_HAND];
			}
			else if (interaction_mode < IM_BASE_1) {
				orientation_ptr = &kits[current_kit_index]->tracker_orientations[interaction_mode - IM_TRACKER_1];
				position_ptr = &kits[current_kit_index]->tracker_positions[interaction_mode - IM_TRACKER_1];
			}
			else {
				orientation_ptr = &base_orientations[interaction_mode - IM_BASE_1];
				position_ptr = &base_positions[interaction_mode - IM_BASE_1];
			}
			if (left_ctrl || right_ctrl) {
				if (is_alt)
					(*position_ptr)[0] += 0.3f * (float)(left_ctrl ? -dt : dt);
				else
					*orientation_ptr = quat(vec3(0, 1, 0), (float)(right_ctrl ? -dt : dt))*(*orientation_ptr);
				update_all_members();
				post_redraw();
			}
			if (up_ctrl || down_ctrl) {
				if (is_alt)
					(*position_ptr)[1] += 0.3f * (float)(down_ctrl ? -dt : dt);
				else
					*orientation_ptr = quat(vec3(1, 0, 0), (float)(up_ctrl ? -dt : dt))*(*orientation_ptr);
				update_all_members();
				post_redraw();
			}
			if (pgup_ctrl || pgdn_ctrl) {
				if (is_alt)
					(*position_ptr)[2] += 0.3f * (float)(pgup_ctrl ? -dt : dt);
				else
					*orientation_ptr = quat(vec3(0, 0, 1), (float)(pgup_ctrl ? -dt : dt))*(*orientation_ptr);
				update_all_members();
				post_redraw();
			}
			if (interaction_mode >= IM_BASE_1) {
				if (is_alt)
					on_set(position_ptr);
				else
					on_set(orientation_ptr);
			}
			break;
		}
		}
	}
}

///
void vr_emulator::on_set(void* member_ptr)
{
	if (member_ptr == &current_kit_index) {
		while (current_kit_index >= (int)kits.size())
			add_new_kit();
	}
	if (!base_serials.empty()) {
		for (int i = 0; i < (int)base_serials.size(); ++i)
			if (member_ptr >= &base_orientations[i] && member_ptr < &base_orientations[i]+1 ||
				member_ptr >= &base_positions[i] && member_ptr < &base_positions[i]+1)
				update_reference_states(i);
	}
	update_member(member_ptr);
	post_redraw();
}

/// return name of driver
std::string vr_emulator::get_driver_name() const
{
	return name;
}

/// return whether driver is installed
bool vr_emulator::is_installed() const
{
	return installed;
}

bool vr_emulator::gamepad_connected = false;

void vr_emulator::add_new_kit()
{
	if (!gamepad_connected) {
		gamepad_connected = true;
		cgv::gui::connect_gamepad_server();
	}
	++counter;
	void* handle = 0;
	(unsigned&)handle = counter;
	vr_emulated_kit* new_kit = new vr_emulated_kit(body_direction, body_position, body_height,
		screen_width, screen_height, this, handle, 
		std::string("vr_emulated_kit[") + cgv::utils::to_string(counter) + "]", nr_trackers);
	kits.push_back(new_kit);
	register_vr_kit(handle, new_kit);
	if (current_kit_index == -1) {
		current_kit_index = int(kits.size()) - 1;
		update_member(&current_kit_index);
	}
	cgv::gui::ref_vr_server().check_device_changes(cgv::gui::trigger::get_current_time());
	post_recreate_gui();
}

/// scan all connected vr kits and return a vector with their ids
std::vector<void*> vr_emulator::scan_vr_kits()
{
	std::vector<void*> result;
	if (is_installed())
		for (auto kit_ptr : kits)
			result.push_back(kit_ptr->get_handle());
	return result;
}

/// scan all connected vr kits and return a vector with their ids
vr::vr_kit* vr_emulator::replace_by_index(int& index, vr::vr_kit* new_kit_ptr)
{
	if (!is_installed())
		return 0;

	for (auto kit_ptr : kits) {
		if (index == 0) {
			replace_vr_kit(kit_ptr->get_handle(), new_kit_ptr);
			return kit_ptr;
		}
		else
			--index;
	}
	return 0;
}
/// scan all connected vr kits and return a vector with their ids
bool vr_emulator::replace_by_pointer(vr::vr_kit* old_kit_ptr, vr::vr_kit* new_kit_ptr)
{
	if (!is_installed())
		return false;

	for (auto kit_ptr : kits) {
		if (kit_ptr == old_kit_ptr || vr::get_vr_kit(kit_ptr->get_handle()) == old_kit_ptr) {
			replace_vr_kit(kit_ptr->get_handle(), new_kit_ptr);
			return true;
		}
	}
	return false;
}

/// put a 3d up direction into passed array
void vr_emulator::put_up_direction(float* up_dir) const
{
	reinterpret_cast<vec3&>(*up_dir) = vec3(0, 1, 0);
}

/// return the floor level relativ to the world origin
float vr_emulator::get_floor_level() const
{
	return 0;
}

/// return height of action zone in meters
float vr_emulator::get_action_zone_height() const
{
	return 2.5f;
}

/// return a vector of floor points defining the action zone boundary as a closed polygon
void vr_emulator::put_action_zone_bounary(std::vector<float>& boundary) const
{
	boundary.resize(18);
	for (unsigned i = 0; i < 6; ++i) {
		float angle = float(2 * M_PI*i / 6);
		vec3 pi(1.5f*cos(angle), 0, 2.5f*sin(angle));
		reinterpret_cast<vec3&>(boundary[3 * i]) = pi;
	}
}
bool vr_emulator::check_for_button_toggle(cgv::gui::key_event& ke, int controller_index, vr::VRButtonStateFlags button, float touch_x, float touch_y)
{
	if (current_kit_index == -1)
		return false;
	if (current_kit_index >= (int)kits.size())
		return false;
	if (ke.get_action() != cgv::gui::KA_PRESS)
		return false;
	if (ke.get_modifiers() == cgv::gui::EM_SHIFT) {
		kits[current_kit_index]->state.controller[controller_index].axes[0] = touch_x;
		kits[current_kit_index]->state.controller[controller_index].axes[1] = touch_y;
	}
	else if (ke.get_modifiers() == 0)
		kits[current_kit_index]->state.controller[controller_index].button_flags ^= button;
	else
		return false;
	update_all_members();
	return true;
}

bool vr_emulator::handle_ctrl_key(cgv::gui::key_event& ke, bool& fst_ctrl, bool* snd_ctrl_ptr)
{
	if (current_kit_index == -1)
		return false;

	if (snd_ctrl_ptr && (ke.get_modifiers() & cgv::gui::EM_SHIFT) != 0) {
		*snd_ctrl_ptr = (ke.get_action() != cgv::gui::KA_RELEASE);
		update_member(snd_ctrl_ptr);
	}
	else {
		fst_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
		update_member(&fst_ctrl);
	}
	is_alt = (ke.get_action() != cgv::gui::KA_RELEASE) && ((ke.get_modifiers() & cgv::gui::EM_ALT) != 0);
	update_member(&is_alt);
	return true;
}

/// overload and implement this method to handle events
bool vr_emulator::handle(cgv::gui::event& e)
{
	if (e.get_kind() != cgv::gui::EID_KEY)
		return false;
	cgv::gui::key_event& ke = static_cast<cgv::gui::key_event&>(e);
	switch (ke.get_key()) {
	case 'N' :
		if (ke.get_action() == cgv::gui::KA_PRESS && 
			ke.get_modifiers() == cgv::gui::EM_CTRL + cgv::gui::EM_ALT) {
			add_new_kit();
			return true;
		}
		return false;
	case '0':
	case '1':
	case '2':
	case '3':
		if (ke.get_modifiers() == cgv::gui::EM_SHIFT) {
			if (ke.get_action() != cgv::gui::KA_RELEASE) {
				current_kit_index = ke.get_key() - '0';
				if (current_kit_index >= (int)kits.size())
					current_kit_index = -1;
				update_member(&current_kit_index);
			}
			return true;
		}		
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		interaction_mode = InteractionMode(IM_BODY + ke.get_key() - '0');
		update_member(&interaction_mode);
		return true;
	case 'Q': return check_for_button_toggle(ke, 0, vr::VRF_SYSTEM, -1, 1);
	case 'W': return check_for_button_toggle(ke, 0, vr::VRF_MENU, 0, 1);
	case 'E': return check_for_button_toggle(ke, 0, vr::VRF_A, 1, 1);

	case 'A': return check_for_button_toggle(ke, 0, vr::VRF_INPUT0_TOUCH, -1, 0);
	case 'S': return check_for_button_toggle(ke, 0, vr::VRF_INPUT0, 0, 0);
	case 'D': return check_for_button_toggle(ke, 0, vr::VRF_INPUT1_TOUCH, 1, 0);

	case 'Y': return check_for_button_toggle(ke, 0, vr::VRF_GRIP, -1, -1);
	case 'X': return check_for_button_toggle(ke, 0, vr::VRF_GRIP, 0, -1);
	case 'C': return check_for_button_toggle(ke, 0, vr::VRF_INPUT1,  1, -1);


	case 'U': return check_for_button_toggle(ke, 1, vr::VRF_SYSTEM, -1, 1);
	case 'I': return check_for_button_toggle(ke, 1, vr::VRF_MENU, 0, 1);
	case 'O': return check_for_button_toggle(ke, 1, vr::VRF_A, 1, 1);

	case 'J': return check_for_button_toggle(ke, 1, vr::VRF_INPUT0_TOUCH, -1, 0);
	case 'K': return check_for_button_toggle(ke, 1, vr::VRF_INPUT0, 0, 0);
	case 'L': return check_for_button_toggle(ke, 1, vr::VRF_INPUT1_TOUCH, 1, 0);

	case 'M': return check_for_button_toggle(ke, 1, vr::VRF_GRIP, -1, -1);
	case ',': return check_for_button_toggle(ke, 1, vr::VRF_GRIP, 0, -1);
	case '.': return check_for_button_toggle(ke, 1, vr::VRF_INPUT1, 1, -1);

	case cgv::gui::KEY_Left:
	case cgv::gui::KEY_Num_4:
		return handle_ctrl_key(ke, left_ctrl, &home_ctrl);
	case cgv::gui::KEY_Right: 
	case cgv::gui::KEY_Num_6:
		return handle_ctrl_key(ke, right_ctrl, &end_ctrl);
	case cgv::gui::KEY_Up: 
	case cgv::gui::KEY_Num_8:
		return handle_ctrl_key(ke, up_ctrl, &pgup_ctrl);
	case cgv::gui::KEY_Down:
	case cgv::gui::KEY_Num_2:
		return handle_ctrl_key(ke, down_ctrl, &pgdn_ctrl);
	case cgv::gui::KEY_Home:
	case cgv::gui::KEY_Num_7:
		return handle_ctrl_key(ke, home_ctrl);
	case cgv::gui::KEY_End:
	case cgv::gui::KEY_Num_1:
		return handle_ctrl_key(ke, end_ctrl);
	case cgv::gui::KEY_Page_Up:
	case cgv::gui::KEY_Num_9:
		return handle_ctrl_key(ke, pgup_ctrl);
	case cgv::gui::KEY_Page_Down:
	case cgv::gui::KEY_Num_3:
		return handle_ctrl_key(ke, pgdn_ctrl);
	}
	return false;
}
/// overload to stream help information to the given output stream
void vr_emulator::stream_help(std::ostream& os)
{
	os << "vr_emulator:\n"
	   << "  Ctrl-Alt-N to create vr kit indexed from 0\n"
	   << "  Shift-<0-3> to select to be controlled vr kit (unselect if key's kit not exits)\n"
	   << "  <0-9> select <body|left hand|right hand|tracker 1|tracker 2|tracker 3|tracker 4|base 1|base 2|base 3> for control\n"
	   << "    body: <up|down> .. move,  <left|right> .. turn, <pgup|pgdn> .. bend, \n"
	   << "          <home|end> .. gear, <alt>+<left|right> .. side step\n"
	   << "    hand: <home|end> .. trigger\n"
	   << "    hand&tracker: <left|right|up|down|pgdn|pgup> .. rotate or with <alt> translate\n"
	   << "  <Q|W|E|A|S|D|Y|X|C:U|I|O|J|K|L|M|,|.> toggle left:right controller buttons\n"
	   << "    <System|Menu|A|Touch0|Press0|Touch1|Grip|Grip|Press1>; adjust TP position with Shift\n"
	   << "  <Shift>-<QWE:ASD:YXC>|<UIO:HJK:BNM> to set left:right controller touch xy to -1|0|+1\n"
	   << std::endl;
}
/// return the type name 
std::string vr_emulator::get_type_name() const
{
	return "vr_emulator";
}
/// overload to show the content of this object
void vr_emulator::stream_stats(std::ostream& os)
{
	static std::string i_modes("body,left hand,right hand,tracker 1,tracker 2,tracker 3,tracker 4,base 1, base 2,base 3, base 4");
	os << "vr_emulator: [" << cgv::utils::get_element(i_modes, (int)interaction_mode, ',') << "]" << std::endl;
}
///
bool vr_emulator::init(cgv::render::context& ctx)
{
	return true;
}
void vr_emulator::init_frame(cgv::render::context& ctx)
{
}
void vr_emulator::draw(cgv::render::context&)
{
	for (auto kit_ptr : kits) {
	}
}
void vr_emulator::finish_frame(cgv::render::context&)
{
}
bool vr_emulator::self_reflect(cgv::reflect::reflection_handler& srh)
{
	bool res =
		srh.reflect_member("current_kit_index", current_kit_index) &&
		srh.reflect_member("installed", installed) &&
		srh.reflect_member("create_body_direction", body_direction) &&
		srh.reflect_member("create_body_position", body_position) &&
		srh.reflect_member("create_nr_trackers", nr_trackers) &&
		srh.reflect_member("body_height", body_height) &&
		srh.reflect_member("screen_height", screen_height) &&
		srh.reflect_member("screen_width", screen_width);
	if (res && current_kit_index != -1 && current_kit_index < (int)kits.size()) {
		vr_emulated_kit* kit_ptr = kits[current_kit_index];
		res =
			srh.reflect_member("body_direction", kit_ptr->body_direction) &&
			srh.reflect_member("body_height", kit_ptr->body_height) &&
			srh.reflect_member("hip_parameter", kit_ptr->hip_parameter) &&
			srh.reflect_member("yaw_parameter", kit_ptr->yaw_parameter) &&
			srh.reflect_member("fovy", kit_ptr->fovy) &&
			srh.reflect_member("body_position", kit_ptr->body_position) &&
			srh.reflect_member("left_hand_position", kit_ptr->hand_position[0]) &&
			srh.reflect_member("right_hand_position", kit_ptr->hand_position[1]) &&
			srh.reflect_member("tracker_1_position", kit_ptr->tracker_positions[0]) &&
			srh.reflect_member("tracker_2_position", kit_ptr->tracker_positions[1]) &&
			srh.reflect_member("tracker_3_position", kit_ptr->tracker_positions[2]) &&
			srh.reflect_member("tracker_4_position", kit_ptr->tracker_positions[3]) &&
			srh.reflect_member("tracker_1_enabled", kit_ptr->tracker_enabled[0]) &&
			srh.reflect_member("tracker_2_enabled", kit_ptr->tracker_enabled[1]) &&
			srh.reflect_member("tracker_3_enabled", kit_ptr->tracker_enabled[2]) &&
			srh.reflect_member("tracker_4_enabled", kit_ptr->tracker_enabled[3]) &&
			srh.reflect_member("tracker_1_orientation", kit_ptr->tracker_orientations[0]) &&
			srh.reflect_member("tracker_2_orientation", kit_ptr->tracker_orientations[1]) &&
			srh.reflect_member("tracker_3_orientation", kit_ptr->tracker_orientations[2]) &&
			srh.reflect_member("tracker_4_orientation", kit_ptr->tracker_orientations[3]) &&
			srh.reflect_member("left_hand_orientation", kit_ptr->hand_orientation[0]) &&
			srh.reflect_member("right_hand_orientation", kit_ptr->hand_orientation[1]) &&
			srh.reflect_member("tracker_1_attachment", (int&)kit_ptr->tracker_attachments[0]) &&
			srh.reflect_member("tracker_2_attachment", (int&)kit_ptr->tracker_attachments[1]) &&
			srh.reflect_member("tracker_3_attachment", (int&)kit_ptr->tracker_attachments[2]) &&
			srh.reflect_member("tracker_4_attachment", (int&)kit_ptr->tracker_attachments[3]);
	}
	return res;
}
void vr_emulator::create_trackable_gui(const std::string& name, vr::vr_trackable_state& ts)
{
	add_decorator(name, "heading", "level=3");
	add_member_control(this, "status", ts.status, "dropdown", "enums='detached,attached,tracked'");
	if (begin_tree_node("pose", ts.pose[0], false, "level=3")) {
		align("\a");
		add_view("x.x", ts.pose[0], "value", "w=50;step=0.0001", " ");
		add_view("x.y", ts.pose[1], "value", "w=50;step=0.0001", " ");
		add_view("x.z", ts.pose[2], "value", "w=50;step=0.0001");
		add_view("y.x", ts.pose[3], "value", "w=50;step=0.0001", " ");
		add_view("y.y", ts.pose[4], "value", "w=50;step=0.0001", " ");
		add_view("y.z", ts.pose[5], "value", "w=50;step=0.0001");
		add_view("z.x", ts.pose[6], "value", "w=50;step=0.0001", " ");
		add_view("z.y", ts.pose[7], "value", "w=50;step=0.0001", " ");
		add_view("z.z", ts.pose[8], "value", "w=50;step=0.0001");
		add_view("0.x", ts.pose[9], "value", "w=50;step=0.0001", " ");
		add_view("0.y", ts.pose[10], "value", "w=50;step=0.0001", " ");
		add_view("0.z", ts.pose[11], "value", "w=50;step=0.0001");
		align("\b");
		end_tree_node(ts.pose[0]);
	}
}
void vr_emulator::create_controller_gui(int i, vr::vr_controller_state& cs)
{
	create_trackable_gui(std::string("controller") + cgv::utils::to_string(i), cs);
	/// a unique time stamp for fast test whether state changed
	add_view("time_stamp", cs.time_stamp);
	add_member_control(this, "touch.x", cs.axes[0], "value_slider", "min=-1;max=1;ticks=true");
	add_member_control(this, "touch.y", cs.axes[1], "value_slider", "min=-1;max=1;ticks=true");
	add_member_control(this, "trigger", cs.axes[2], "value_slider", "min=0;max=1;ticks=true");
	//add_view("axes[3]", cs.axes[3], "value", "w=50", " ");
	//add_view("axes[4]", cs.axes[4], "value", "w=50", " ");
	//add_view("axes[5]", cs.axes[5], "value", "w=50");
	//add_view("axes[6]", cs.axes[6], "value", "w=50", " ");
	//add_view("axes[7]", cs.axes[7], "value", "w=50");
	add_view("vibration[0]", cs.vibration[0], "value", "w=50", " ");
	add_view("[1]", cs.vibration[1], "value", "w=50");
}
void vr_emulator::create_tracker_gui(vr_emulated_kit* kit, int i)
{
	if (begin_tree_node(std::string("tracker_") + cgv::utils::to_string(i+1), kit->tracker_enabled[i], false, "level=3")) {
		add_member_control(this, "enabled", kit->tracker_enabled[i], "check");
		add_member_control(this, "attachment", kit->tracker_attachments[i], "dropdown", "enums='world,head,left hand,right hand'");
		add_decorator("position", "heading", "level=3");
		add_gui("position", kit->tracker_positions[i], "vector", "gui_type='value_slider';options='min=-3;max=3;step=0.0001;ticks=true'");
		add_decorator("orientation", "heading", "level=3");
		add_gui("orientation", reinterpret_cast<vec4&>(kit->tracker_orientations[i]), "direction", "gui_type='value_slider';options='min=-1;max=1;step=0.0001;ticks=true'");
		end_tree_node(kit->tracker_enabled[i]);
	}
}
void vr_emulator::create_gui()
{
	add_decorator("vr emulator", "heading", "level=2");
	add_member_control(this, "installed", installed, "check");
	if (begin_tree_node("base and calib", coordinate_rotation, false, "level=2")) {
		align("\a");
		add_decorator("coordinate transform", "heading", "level=3");
		add_gui("coordinate_rotation", reinterpret_cast<vec4&>(coordinate_rotation), "direction", "options='min=-1;max=1;step=0.0001;ticks=true'");
		add_gui("coordinate_displacement", coordinate_displacement, "", "options='min=-2;max=2;step=0.0001;ticks=true'");
		add_decorator("base stations", "heading", "level=3");
		for (uint32_t i = 0; i < base_serials.size(); ++i) {
			add_member_control(this, "serial", base_serials[i]);
			add_gui("orientation", reinterpret_cast<vec4&>(base_orientations[i]), "direction", "options='min=-1;max=1;step=0.0001;ticks=true'");
			add_gui("position", base_positions[i], "", "options='min=-2;max=2;step=0.0001;ticks=true'");
		}
		align("\b");
		end_tree_node(coordinate_rotation);
	}
	if (begin_tree_node("create kit", screen_width, false, "level=2")) {
		align("\a");
		add_member_control(this, "screen_width", screen_width, "value_slider", "min=320;max=1920;ticks=true");
		add_member_control(this, "screen_height", screen_height, "value_slider", "min=240;max=1920;ticks=true");
		add_gui("body_position", body_position, "", "options='min=-1;max=1;step=0.0001;ticks=true'");
		add_member_control(this, "body_direction", body_direction, "min=0;max=6.3;ticks=true");
		add_member_control(this, "body_height", body_height, "min=1.2;max=2.0;step=0.001;ticks=true");
		add_member_control(this, "nr_trackers", nr_trackers, "min=0;max=4;ticks=true");
		connect_copy(add_button("create new kit")->click, cgv::signal::rebind(this, &vr_emulator::add_new_kit));
		align("\b");
		end_tree_node(screen_width);
	}
	add_view("current_kit", current_kit_index, "", "w=50", " ");
	add_member_control(this, "mode", interaction_mode, "dropdown", "w=100;enums='body,left hand,right hand,tracker 1,tracker 2,tracker 3,tracker 4,base 1, base 2,base 3, base 4'");
	add_member_control(this, "alt", is_alt,   "toggle", "w=15", " ");
	add_member_control(this, "L", left_ctrl,  "toggle", "w=15", "");
	add_member_control(this, "R", right_ctrl, "toggle", "w=15", " ");
	add_member_control(this, "U", up_ctrl,    "toggle", "w=15", "");
	add_member_control(this, "D", down_ctrl,  "toggle", "w=15", " ");
	add_member_control(this, "H", home_ctrl,  "toggle", "w=15", "");
	add_member_control(this, "E", end_ctrl,   "toggle", "w=15", " ");
	add_member_control(this, "P", pgup_ctrl,  "toggle", "w=15", "");
	add_member_control(this, "p", pgdn_ctrl,  "toggle", "w=15");

	for (unsigned i = 0; i < kits.size(); ++i) {
		if (begin_tree_node(kits[i]->get_name(), *kits[i], true, "level=2")) {
			align("\a");
			add_view("buttons left", kits[i]->fovy, "", "w=0", "");
			add_gui("button_flags", kits[i]->state.controller[0].button_flags, "bit_field_control",
				"enums='SY=1,ME=2,GR=4,A=128,PT=256,PP=512,TT=1024,TP=2048';options='w=16;tooltip=\""
				"SYstem button<Q> \nMEnu button <W>\nA button <E>\n"
				"Pad Touch <A>\nPad Press <S>\nTrigger Touch\n<D>"
				"GRip button <Y|X>\nTrigger Press <C>\"';"
				"align='';gui_type='toggle'");
			align(" ");
			add_view("Pxy", kits[i]->state.controller[0].axes[0], "", "w=16", "");
			add_view("", kits[i]->state.controller[0].axes[1], "", "w=16");
			add_view("buttons right", kits[i]->fovy, "", "w=0", "");
			add_gui("button_flags", kits[i]->state.controller[1].button_flags, "bit_field_control",
				"enums='SY=1,ME=2,GR=4,A=128,PT=256,PP=512,TT=1024,TP=2048';options='w=16;tooltip=\""
				"SYstem button<U>\nMEnu button <I>\nA button <O>\n"
				"Pad Touch <J>\nPad Press <K>\nTrigger Touch\n"
				"GRip button <M|,>\nTrigger Press\"';"
				"align='';gui_type='toggle'");			
			align(" ");
			add_view("Pxy", kits[i]->state.controller[1].axes[0], "", "w=16", "");
			add_view("", kits[i]->state.controller[1].axes[1], "", "w=16");

			add_member_control(this, "fovy", kits[i]->fovy, "value_slider", "min=30;max=180;ticks=true;log=true");
			if (begin_tree_node("body pose", kits[i]->body_position, false, "level=3")) {
				align("\a");
				add_decorator("body position", "heading", "level=3");
				add_gui("body_position", kits[i]->body_position, "", "options='min=-5;max=5;ticks=true'");
				add_member_control(this, "body_direction", kits[i]->body_direction, "value_slider", "min=0;max=6.3;ticks=true");
				add_member_control(this, "body_height", kits[i]->body_height, "value_slider", "min=1.2;max=2.0;ticks=true");
				add_member_control(this, "hip_parameter", kits[i]->hip_parameter, "value_slider", "min=-1;max=1;step=0.0001;ticks=true");
				add_member_control(this, "yaw_parameter", kits[i]->yaw_parameter, "value_slider", "min=-1;max=1;step=0.0001;ticks=true");
				for (int j = 0; j < 2; ++j) {
					if (begin_tree_node(j == 0 ? "left hand" : "right hand", kits[i]->hand_position[j], false, "level=3")) {
						align("\a");
						add_decorator("position", "heading", "level=3");
						add_gui("position", kits[i]->hand_position[j], "vector", "gui_type='value_slider';options='min=-3;max=3;step=0.0001;ticks=true'");
						add_decorator("orientation", "heading", "level=3");
						add_gui("orientation", reinterpret_cast<vec4&>(kits[i]->hand_orientation[j]), "direction", "gui_type='value_slider';options='min=-1;max=1;step=0.0001;ticks=true'");
						align("\b");
						end_tree_node(kits[i]->hand_position[j]);
					}
				}
				align("\b");
				end_tree_node(kits[i]->body_position);
			}
			create_tracker_gui(kits[i], 0);
			create_tracker_gui(kits[i], 1);
			create_tracker_gui(kits[i], 2);
			create_tracker_gui(kits[i], 3);
			if (begin_tree_node("state", kits[i]->state.controller[0].pose[1], false, "level=3")) {
				align("\a");
				if (begin_tree_node("hmd", kits[i]->state.hmd.pose[0], false, "level=3")) {
					align("\a");
						create_trackable_gui("hmd", kits[i]->state.hmd);
					align("\b");
					end_tree_node(kits[i]->state.hmd.pose[1]);
				}
				for (unsigned ci = 0; ci < 2; ++ci) {
					if (begin_tree_node((ci==0?"left controller":"right controller"), kits[i]->state.controller[ci], false, "level=3")) {
						align("\a");
						create_controller_gui(ci, kits[i]->state.controller[ci]);
						align("\b");
						end_tree_node(kits[i]->state.controller[ci]);
					}
				}
				align("\b");
				end_tree_node(kits[i]->state.controller[0].pose[1]);
			}
			align("\b");
			end_tree_node(*kits[i]);
		}
	}
}

struct register_driver_and_object
{
	register_driver_and_object(const std::string& options)
	{
		vr_emulator* vr_emu_ptr = new vr_emulator();
		vr::register_driver(vr_emu_ptr);
		register_object(base_ptr(vr_emu_ptr), options);
	}
};

register_driver_and_object vr_emu_reg("vr_test");