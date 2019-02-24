#include "vr_emulator.h"
#include <cgv/math/ftransform.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/trigger.h>

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

vr_emulated_kit::mat3x4 vr_emulated_kit::construct_pos_matrix(const quat& orientation, const vec3& position)
{
	mat3 R;
	orientation.put_matrix(R);
	mat3x4 P;
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
		cgv::math::rotate4<float>(-90*gear_parameter, vec3(0, 1, 0));

	mat4 T_left =
		cgv::math::translate4<float>(
			scale*vec3(-(Shoulder_breadth + Arm_length * hand_position[0](0)),
				Shoulder_height - Hip_height + Arm_length * hand_position[0](1),
				Arm_length*hand_position[0](2)));
	mat4 T_right =
		cgv::math::translate4<float>(
			scale*vec3(+(Shoulder_breadth + Arm_length * hand_position[1](0)),
				Shoulder_height - Hip_height + Arm_length * hand_position[1](1),
				Arm_length*hand_position[1](2)));

	set_pose_matrix(T_body*T_hip*T_head, state.hmd.pose);
	set_pose_matrix(T_body*T_hip*T_left, state.controller[0].pose);
	set_pose_matrix(T_body*T_hip*T_right, state.controller[1].pose);
}


vr_emulated_kit::vr_emulated_kit(float _body_direction, const vec3& _body_position, float _body_height, unsigned _width, unsigned _height, vr::vr_driver* _driver, void* _handle, const std::string& _name, bool _ffb_support, bool _wireless)
	: gl_vr_display(_width, _height, _driver, _handle, _name, _ffb_support, _wireless)
{

	body_position = _body_position;
	body_direction=_body_direction;
	body_height = _body_height;
	hip_parameter= 0;
	gear_parameter = 0;
	fovy = 90;
	hand_position[0] = vec3(0, -0.5f, -0.2f);
	hand_position[1] = vec3(0, -0.5f, -0.2f);
	state.hmd.status = vr::VRS_TRACKED;
	state.controller[0].status = vr::VRS_TRACKED;
	state.controller[1].status = vr::VRS_TRACKED;
	
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


void vr_emulated_kit::set_pose_matrix(const mat4& H, float* pose)
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

bool vr_emulated_kit::query_state(vr::vr_kit_state& state, int pose_query)
{
	compute_state_poses();
	state = this->state;
	return true;
}

bool vr_emulated_kit::set_vibration(unsigned controller_index, float low_frequency_strength, float high_frequency_strength)
{
	state.controller[controller_index].vibration[0] = low_frequency_strength;
	state.controller[controller_index].vibration[1] = high_frequency_strength;
	return has_force_feedback();
}

void vr_emulated_kit::put_eye_to_head_matrix(int eye, float* pose_matrix)
{
	float scale = body_height / Body_height;
	set_pose_matrix(
		cgv::math::translate4<float>(
			scale*vec3(float(2 * eye - 1)*Pupillary_distance, Eye_height - Chin_height, -Pupillary_distance)
		), pose_matrix
	);
}

void vr_emulated_kit::put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix)
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
	left_ctrl = right_ctrl = up_ctrl = down_ctrl = false;
	home_ctrl = end_ctrl = pgup_ctrl = pgdn_ctrl = false;
	current_kit_ctrl = -1;
	installed = true;
	body_speed = 1.0f;
	body_position = vec3(0,0,1);
	body_height = 1.75f;
	body_direction = 0;
	screen_width = 640;
	screen_height = 480;
	ffb_support = true;
	wireless = false;
	counter = 0;
	connect(cgv::gui::get_animation_trigger().shoot, this, &vr_emulator::timer_event);
}

void vr_emulator::timer_event(double t, double dt)
{
	if (current_kit_ctrl >= 0 && current_kit_ctrl < (int)kits.size()) {
		if (left_ctrl || right_ctrl) {
			kits[current_kit_ctrl]->body_direction += 3 * (float)(left_ctrl ? -dt : dt);
			update_all_members();
			post_redraw();
		}
		if (up_ctrl || down_ctrl) {
			kits[current_kit_ctrl]->body_position -= 3 * (float)(down_ctrl ? -dt : dt) * kits[current_kit_ctrl]->get_body_direction();
			update_all_members();
			post_redraw();
		}
		if (home_ctrl || end_ctrl) {
			kits[current_kit_ctrl]->gear_parameter += (float)(home_ctrl ? -dt : dt);
			if (kits[current_kit_ctrl]->gear_parameter < -1)
				kits[current_kit_ctrl]->gear_parameter = -1;
			else if (kits[current_kit_ctrl]->gear_parameter > 1)
				kits[current_kit_ctrl]->gear_parameter = 1;
			update_all_members();
			post_redraw();
		}
		if (pgup_ctrl || pgdn_ctrl) {
			kits[current_kit_ctrl]->hip_parameter += (float)(pgup_ctrl ? -dt : dt) ;
			if (kits[current_kit_ctrl]->hip_parameter < -1)
				kits[current_kit_ctrl]->hip_parameter = -1;
			else if (kits[current_kit_ctrl]->hip_parameter > 1)
				kits[current_kit_ctrl]->hip_parameter = 1;
			update_all_members();
			post_redraw();
		}
	}
}

///
void vr_emulator::on_set(void* member_ptr)
{
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

void vr_emulator::add_new_kit()
{
	++counter;
	void* handle = 0;
	(unsigned&)handle = counter;
	vr_emulated_kit* new_kit = new vr_emulated_kit(body_direction, body_position, body_height,
		screen_width, screen_height, this, handle, 
		std::string("vr_emulated_kit[") + cgv::utils::to_string(counter) + "]", ffb_support, wireless);
	kits.push_back(new_kit);
	register_vr_kit(handle, new_kit);
	post_recreate_gui();
}

/// scan all connected vr kits and return a vector with their ids
std::vector<void*> vr_emulator::scan_vr_kits()
{
	std::vector<void*> result;

	if (is_installed())
		for (auto kit_ptr : kits)
			result.push_back(kit_ptr->get_device_handle());
	return result;
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
bool vr_emulator::check_for_button_toggle(cgv::gui::key_event& ke, int controller_index, vr::VRButtonStateFlags button)
{
	if (current_kit_ctrl == -1)
		return false;
	if (current_kit_ctrl >= (int)kits.size())
		return false;
	if (ke.get_action() != cgv::gui::KA_PRESS)
		return false;
	kits[current_kit_ctrl]->state.controller[0].button_flags ^= button;
	update_all_members();
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
		return check_for_button_toggle(ke, 1, vr::VRF_STICK_TOUCH);
	case '0':
	case '1':
	case '2':
	case '3':
		if (ke.get_modifiers() == 0) {
			if (ke.get_action() != cgv::gui::KA_RELEASE)
				current_kit_ctrl = ke.get_key() - '0';
			else
				current_kit_ctrl = -1;
			update_member(&current_kit_ctrl);
			return true;
		}
		break;
	case 'Q': return check_for_button_toggle(ke, 0, vr::VRF_MENU);
	case 'A': return check_for_button_toggle(ke, 0, vr::VRF_BUTTON0);
	case 'D': return check_for_button_toggle(ke, 0, vr::VRF_BUTTON1);
	case 'W': return check_for_button_toggle(ke, 0, vr::VRF_BUTTON2);
	case 'X': return check_for_button_toggle(ke, 0, vr::VRF_BUTTON3);
	case 'C': return check_for_button_toggle(ke, 0, vr::VRF_STICK_TOUCH);
	case 'S': return check_for_button_toggle(ke, 0, vr::VRF_STICK);

	case 'O': return check_for_button_toggle(ke, 1, vr::VRF_MENU);
	case 'L': return check_for_button_toggle(ke, 1, vr::VRF_BUTTON0);
	case 'I': return check_for_button_toggle(ke, 1, vr::VRF_BUTTON1);
	case 'J': return check_for_button_toggle(ke, 1, vr::VRF_BUTTON2);
	case 'M': return check_for_button_toggle(ke, 1, vr::VRF_BUTTON3);
	case 'K': return check_for_button_toggle(ke, 1, vr::VRF_STICK);

	case cgv::gui::KEY_Left:
		if (current_kit_ctrl != -1) {
			left_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
			update_member(&left_ctrl);
			return true;
		}
		break;
	case cgv::gui::KEY_Right: 
		if (current_kit_ctrl != -1) {
			right_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
			update_member(&right_ctrl);
			return true;
		}
		break;
	case cgv::gui::KEY_Up:
		if (current_kit_ctrl != -1) {
			up_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
			update_member(&up_ctrl);
			return true;
		}
		break;
	case cgv::gui::KEY_Down:
		if (current_kit_ctrl != -1) {
			down_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
			update_member(&down_ctrl);
			return true;
		}
		break;
	case cgv::gui::KEY_Home:
		if (current_kit_ctrl != -1) {
			home_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
			update_member(&home_ctrl);
			return true;
		}
		break;
	case cgv::gui::KEY_End:
		if (current_kit_ctrl != -1) {
			end_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
			update_member(&end_ctrl);
			return true;
		}
		break;
	case cgv::gui::KEY_Page_Up:
		if (current_kit_ctrl != -1) {
			pgup_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
			update_member(&pgup_ctrl);
			return true;
		}
		break;
	case cgv::gui::KEY_Page_Down:
		if (current_kit_ctrl != -1) {
			pgdn_ctrl = (ke.get_action() != cgv::gui::KA_RELEASE);
			update_member(&pgdn_ctrl);
			return true;
		}
		break;
	}
	return false;
}
/// overload to stream help information to the given output stream
void vr_emulator::stream_help(std::ostream& os)
{
	os << "vr_emulator:\n   Ctrl-Alt-N to create vr kit\n   press and hold <0|1|2|3> to select vr kit and use arrow keys to move or\n      Q,A,W,D,X,C,S|O,L,I,J,M,K,N to toggle left|right controller buttons";
}
/// return the type name 
std::string vr_emulator::get_type_name() const
{
	return "vr_emulator";
}
/// overload to show the content of this object
void vr_emulator::stream_stats(std::ostream&)
{

}
///
bool vr_emulator::init(cgv::render::context& ctx)
{
	return true;
}

/// this method is called in one pass over all drawables before the draw method
void vr_emulator::init_frame(cgv::render::context& ctx)
{
}

/// 
void vr_emulator::draw(cgv::render::context&)
{
	for (auto kit_ptr : kits) {
	}
}

/// this method is called in one pass over all drawables after drawing
void vr_emulator::finish_frame(cgv::render::context&)
{
}
///
bool vr_emulator::self_reflect(cgv::reflect::reflection_handler& srh)
{
	return
		srh.reflect_member("installed", installed) &&
		srh.reflect_member("body_direction", body_direction) &&
		srh.reflect_member("body_position", body_position) &&
		srh.reflect_member("body_height", body_height) &&
		srh.reflect_member("screen_height", screen_height) &&
		srh.reflect_member("screen_width", screen_width) &&
		srh.reflect_member("wireless", wireless) &&
		srh.reflect_member("ffb_support", ffb_support);
}

void vr_emulator::create_trackable_gui(const std::string& name, vr::vr_trackable_state& ts)
{
	add_decorator(name, "heading", "level=3");
	add_member_control(this, "status", ts.status, "dropdown", "enums='detached,attached,tracked'");
	if (begin_tree_node("pose", ts.pose[0], false, "level=3")) {
		align("\a");
		add_view("x.x", ts.pose[0], "value", "w=50", " ");
		add_view("x.y", ts.pose[1], "value", "w=50", " ");
		add_view("x.z", ts.pose[2], "value", "w=50");
		add_view("y.x", ts.pose[3], "value", "w=50", " ");
		add_view("y.y", ts.pose[4], "value", "w=50", " ");
		add_view("y.z", ts.pose[5], "value", "w=50");
		add_view("z.x", ts.pose[6], "value", "w=50", " ");
		add_view("z.y", ts.pose[7], "value", "w=50", " ");
		add_view("z.z", ts.pose[8], "value", "w=50");
		add_view("0.x", ts.pose[9], "value", "w=50", " ");
		add_view("0.y", ts.pose[10], "value", "w=50", " ");
		add_view("0.z", ts.pose[11], "value", "w=50");
		align("\b");
		end_tree_node(ts.pose[0]);
	}
}

void vr_emulator::create_controller_gui(int i, vr::vr_controller_state& cs)
{
	create_trackable_gui(std::string("controller") + cgv::utils::to_string(i), cs);
	/// a unique time stamp for fast test whether state changed
	add_view("time_stamp", cs.time_stamp);
	add_gui("button_flags", cs.button_flags, "bit_field_control", 
		"enums='menu=1,but0=2,but1=4,but2=8,but3=16,touch=32,stick=64';options='w=35';align='';gui_type='toggle'");
	align("\n");
	add_member_control(this, "touch.x", cs.axes[0], "value_slider", "min=-1;max=1;ticks=true");
	add_member_control(this, "touch.y", cs.axes[1], "value_slider", "min=-1;max=1;ticks=true");
	add_member_control(this, "trigger", cs.axes[2], "value_slider", "min=0;max=1;ticks=true");
	add_view("axes[3]", cs.axes[3], "value", "w=50", " ");
	add_view("axes[4]", cs.axes[4], "value", "w=50", " ");
	add_view("axes[5]", cs.axes[5], "value", "w=50");
	add_view("axes[6]", cs.axes[6], "value", "w=50", " ");
	add_view("axes[7]", cs.axes[7], "value", "w=50");
	add_view("vibration[0]", cs.vibration[0], "value", "w=50", " ");
	add_view("[1]", cs.vibration[1], "value", "w=50");
}

///
void vr_emulator::create_gui()
{
	add_decorator("vr emulator", "heading", "level=2");
	add_member_control(this, "installed", installed, "check");
	if (begin_tree_node("create kit", screen_width, false, "level=2")) {
		align("\a");
		add_member_control(this, "screen_width", screen_width, "value_slider", "min=320;max=1920;ticks=true");
		add_member_control(this, "screen_height", screen_height, "value_slider", "min=240;max=1920;ticks=true");
		add_member_control(this, "ffb_support", ffb_support, "toggle", "w=90", " ");
		add_member_control(this, "wireless", wireless, "toggle", "w=90");
		add_gui("body_position", body_position, "", "options='min=-1;max=1'");
		add_member_control(this, "body_direction", body_direction, "min=0;max=6.3;ticks=true");
		add_member_control(this, "body_height", body_height, "min=1.2;max=2.0;ticks=true");
		connect_copy(add_button("create new kit")->click, cgv::signal::rebind(this, &vr_emulator::add_new_kit));
		align("\b");
		end_tree_node(screen_width);
	}
	add_view("current_kit_ctrl", current_kit_ctrl, "", "w=50", " ");
	add_member_control(this, "left", left_ctrl, "toggle", "w=35", " ");
	add_member_control(this, "right", right_ctrl, "toggle", "w=35", " ");
	add_member_control(this, "up", up_ctrl, "toggle", "w=35", " ");
	add_member_control(this, "down", down_ctrl, "toggle", "w=35");
	align("       ");
	add_member_control(this, "home", home_ctrl, "toggle", "w=35", " ");
	add_member_control(this, "end", end_ctrl, "toggle", "w=35", " ");
	add_member_control(this, "pgup", pgup_ctrl, "toggle", "w=35", " ");
	add_member_control(this, "pgdn", pgdn_ctrl, "toggle", "w=35");

	for (unsigned i = 0; i < kits.size(); ++i) {
		if (begin_tree_node(kits[i]->get_name(), *kits[i], false, "level=2")) {
			align("\a");
			add_member_control(this, "fovy", kits[i]->fovy, "value_slider", "min=30;max=180;ticks=true;log=true");
			if (begin_tree_node("body pose", kits[i]->body_position, false, "level=3")) {
				align("\a");
				add_decorator("body position", "heading", "level=3");
				add_gui("body_position", kits[i]->body_position, "", "options='min=-5;max=5;ticks=true'");
				add_member_control(this, "body_direction", kits[i]->body_direction, "value_slider", "min=0;max=6.3;ticks=true");
				add_member_control(this, "body_height", kits[i]->body_height, "value_slider", "min=1.2;max=2.0;ticks=true");
				add_member_control(this, "hip_parameter", kits[i]->hip_parameter, "value_slider", "min=-1;max=1;ticks=true");
				add_member_control(this, "gear_parameter", kits[i]->gear_parameter, "value_slider", "min=-1;max=1;ticks=true");
				add_decorator("left hand", "heading", "level=3");
				add_gui("left_hand", kits[i]->hand_position[0], "", "options='min=-1;max=1;ticks=true'");
				add_decorator("right hand", "heading", "level=3");
				add_gui("right_hand", kits[i]->hand_position[1], "", "options='min=-1;max=1;ticks=true'");
				align("\b");
				end_tree_node(kits[i]->body_position);
			}
			if (begin_tree_node("state", kits[i]->state.controller[0].pose[1], false, "level=3")) {
				align("\a");
				if (begin_tree_node("hmd", kits[i]->state.hmd.pose[0], false, "level=3")) {
					align("\a");
						create_trackable_gui("hmd", kits[i]->state.hmd);
					align("\b");
					end_tree_node(kits[i]->state.hmd.pose[1]);
				}
				for (unsigned ci = 0; ci < 2; ++ci) {
					if (begin_tree_node((ci==0?"controller[0]":"controller[1]"), kits[i]->state.controller[ci], false, "level=3")) {
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