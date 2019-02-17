#include "vr_emulator.h"
#include <cgv/math/ftransform.h>

const float Body_height = 1740.0f;
const float Eye_height = 1630.0f;
const float Shoulder_height = 1425.0f;
const float Shoulder_width = 465.0f;
const float Arm_span = 1790.0f;
const float Hip_width = 360.0f;
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

vr_emulated_kit::vr_emulated_kit(const quat& _body_orientation, const vec3& _body_position, float _body_height, unsigned _width, unsigned _height, vr::vr_driver* _driver, void* _handle, const std::string& _name, bool _ffb_support, bool _wireless)
	: gl_vr_display(_width, _height, _driver, _handle, _name, _ffb_support, _wireless)
{
	vec3 up_dir;
	driver->put_up_direction(&up_dir(0));
	vec3 x_dir = _body_orientation.apply(vec3(1, 0, 0));
	head_position = _body_position + (0.75f*_body_height)*up_dir;
	hand_position[0] = _body_position + (0.5f*_body_height)*up_dir - (0.2f*_body_height)*x_dir;
	hand_position[1] = _body_position + (0.5f*_body_height)*up_dir + (0.2f*_body_height)*x_dir;
	head_orientation = hand_orientation[0] = hand_orientation[1] = _body_orientation;
	state.hmd.status = vr::VRS_TRACKED;
	state.controller[0].status = vr::VRS_TRACKED;
	state.controller[1].status = vr::VRS_TRACKED;
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
	set_pose_matrix(construct_homogeneous_matrix(head_orientation, head_position), state.hmd.pose);
	set_pose_matrix(construct_homogeneous_matrix(hand_orientation[0], hand_position[0]), state.controller[0].pose);
	set_pose_matrix(construct_homogeneous_matrix(hand_orientation[1], hand_position[1]), state.controller[1].pose);
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
	set_pose_matrix(cgv::math::translate4<float>(-float(2*eye-1)*0.04f, -0.03f, 0.05f), pose_matrix);
}
void vr_emulated_kit::put_projection_matrix(int eye, float z_near, float z_far, float* projection_matrix)
{
	reinterpret_cast<mat4&>(*projection_matrix) = 
		cgv::math::perspective4<float>(45.0f, float(width)/height, z_near, z_far);
}

void vr_emulated_kit::submit_frame()
{

}

///
vr_emulator::vr_emulator() : cgv::base::node("vr_emulator")
{
	installed = true;
	body_speed = 1.0f;
	body_position.zeros();
	body_height = 1.75f;
	body_orientation.set(vec3(1, 0, 0), 0);
	screen_width = 640;
	screen_height = 480;
	ffb_support = true;
	wireless = false;
	counter = 0;
}

/// return name of driver
std::string vr_emulator::get_driver_name()
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
	vr_emulated_kit* new_kit = new vr_emulated_kit(body_orientation, body_position, body_height,
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
void vr_emulator::put_up_direction(float* up_dir)
{
	reinterpret_cast<vec3&>(*up_dir) = vec3(0, 1, 0);
}

/// return the floor level relativ to the world origin
float vr_emulator::get_floor_level()
{
	return 0;
}

/// return height of interaction zone in meters
float vr_emulator::get_interaction_zone_height()
{
	return 2.5f;
}

/// return a vector of floor points defining the interaction zone boundary as a closed polygon
void vr_emulator::put_interaction_zone_bounary(std::vector<float>& boundary)
{
	boundary.resize(24);
	for (unsigned i = 0; i < 6; ++i) {
		float angle = float(2 * M_PI*i / 6);
		vec3 pi(cos(angle), 0, sin(angle));
		reinterpret_cast<vec3&>(boundary[3 * i]) = pi;
	}
}

/// overload and implement this method to handle events
bool vr_emulator::handle(cgv::gui::event& e)
{
	return false;
}
/// overload to stream help information to the given output stream
void vr_emulator::stream_help(std::ostream& os)
{

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
		srh.reflect_member("screen_height", screen_height) &&
		srh.reflect_member("screen_width", screen_width) &&
		srh.reflect_member("wireless", wireless) &&
		srh.reflect_member("ffb_support", ffb_support);
}

void vr_emulator::create_trackable_gui(const std::string& name, vr::vr_trackable_state& ts)
{
	add_decorator(name, "heading", "level=4");
	add_member_control(this, "status", ts.status, "dropdown", "enums='detached,attached,tracked'");
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
}

void vr_emulator::create_controller_gui(int i, vr::vr_controller_state& cs)
{
	create_trackable_gui(std::string("controller") + cgv::utils::to_string(i), cs);
	/// a unique time stamp for fast test whether state changed
	add_view("time_stamp", cs.time_stamp);
	add_gui("button_flags", cs.button_flags, "bit_field_control", "enums='menu=1,button0=2,button1=4,button2=8,button3=16,touch=32,stick=64'");
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
	add_decorator("create", "heading", "level=3");
	add_member_control(this, "screen_width", screen_width, "value_slider", "min=320;max=1920;ticks=true");
	add_member_control(this, "screen_height", screen_height, "value_slider", "min=240;max=1920;ticks=true");
	add_member_control(this, "ffb_support", ffb_support, "toggle");
	add_member_control(this, "wireless", wireless, "toggle");
	connect_copy(add_button("create new kit")->click, cgv::signal::rebind(this, &vr_emulator::add_new_kit));
	for (unsigned i = 0; i < kits.size(); ++i) {
		add_decorator(kits[i]->get_name(), "heading", "level=3");
		create_trackable_gui("hmd", kits[i]->state.hmd);
		create_controller_gui(0, kits[i]->state.controller[0]);
		create_controller_gui(1, kits[i]->state.controller[1]);
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