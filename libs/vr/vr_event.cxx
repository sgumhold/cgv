#include "vr_event.h"

namespace vr {
	/// standard constructor for initialization of members
	vr_trackable_state::vr_trackable_state()
	{
		status = VRS_DETACHED;
		pose[0] = pose[4] = pose[8] = 1;
		pose[1] = pose[2] = pose[3] = pose[5] = pose[6] = pose[7] = 0;
		pose[9] = pose[10] = pose[11] = 0;
	}
	/// standard constructor for initialization of members
	vr_controller_state::vr_controller_state()
	{
		time_stamp = 0;
		button_flags = 0;
		for (unsigned i = 0; i < 8; ++i)
			axes[i] = 0;
		vibration[0] = vibration[1] = 0;
	}

	vr_kit_state::vr_kit_state()
	{

	}
	/// check for equality
	bool vr_kit_state::operator == (const vr_kit_state& state) const
	{
		return hmd == state.hmd &&
			controller[0] == state.controller[0] &&
			controller[1] == state.controller[1];
	}
	///
	bool vr_trackable_state::operator == (const vr_trackable_state& state) const
	{
		return status == state.status &&
			pose[0] == state.pose[0] &&
			pose[1] == state.pose[1] &&
			pose[2] == state.pose[2] &&
			pose[3] == state.pose[3] &&
			pose[4] == state.pose[4] &&
			pose[5] == state.pose[5] &&
			pose[6] == state.pose[6] &&
			pose[7] == state.pose[7] &&
			pose[8] == state.pose[8] &&
			pose[9] == state.pose[9] &&
			pose[10] == state.pose[10] &&
			pose[11] == state.pose[11];
	}
	///
	bool vr_controller_state::operator == (const vr_controller_state& state) const
	{
		return
			vr_trackable_state::operator==(state) &&
			button_flags == state.button_flags &&
			axes[0] == state.axes[0] &&
			axes[1] == state.axes[1] &&
			axes[2] == state.axes[2] &&
			axes[3] == state.axes[3] &&
			axes[4] == state.axes[4] &&
			axes[5] == state.axes[5] &&
			axes[6] == state.axes[6] &&
			axes[7] == state.axes[7] &&
			vibration[0] == state.vibration[0] &&
			vibration[1] == state.vibration[1];
	}
	/// convert key to string
	std::string get_key_string(unsigned short key)
	{
		static const char* vr_key_names[] = {
		"VR_UNKNOWN",
		"VR_L_MENU",
		"VR_L_BUTTON0",
		"VR_L_BUTTON1",
		"VR_L_BUTTON2",
		"VR_L_BUTTON3",
		"VR_L_TRIGGER",
		"VR_L_STICK_PRESS",
		"VR_L_STICK_UP",
		"VR_L_STICK_DOWN",
		"VR_L_STICK_RIGHT",
		"VR_L_STICK_LEFT",
		"VR_L_STICK_UPLEFT",
		"VR_L_STICK_UPRIGHT",
		"VR_L_STICK_DOWNRIGHT",
		"VR_L_STICK_DOWNLEFT",
		"VR_R_MENU",
		"VR_R_BUTTON0",
		"VR_R_BUTTON1",
		"VR_R_BUTTON2",
		"VR_R_BUTTON3",
		"VR_R_TRIGGER",
		"VR_R_STICK_PRESS",
		"VR_R_STICK_UP",
		"VR_R_STICK_DOWN",
		"VR_R_STICK_RIGHT",
		"VR_R_STICK_LEFT",
		"VR_R_STICK_UPLEFT",
		"VR_R_STICK_UPRIGHT",
		"VR_R_STICK_DOWNRIGHT",
		"VR_R_STICK_DOWNLEFT"
		};
		int index = key - (int)VR_UNKNOWN;
		return index < 31 ? vr_key_names[index] : "VR_UNKNOWN";
	}
	/// convert flags to string
	std::string get_state_flag_string(VRButtonStateFlags flags)
	{
		static const char* flag_names[] = {
		"MENU",
		"BUTTON0",
		"BUTTON1",
		"BUTTON2",
		"BUTTON3",
		"TOUCH",
		"PRESS"
		};
		static const VRButtonStateFlags flag_values[] = {
			VRF_MENU,
			VRF_BUTTON0,
			VRF_BUTTON1,
			VRF_BUTTON2,
			VRF_BUTTON3,
			VRF_STICK_TOUCH,
			VRF_STICK
		};
		std::string result;
		for (unsigned i = 0; i < 7; ++i)
			if ((flags & flag_values[i]) != 0) {
				if (result.empty())
					result = flag_names[i];
				else
					result += std::string("+") + flag_names[i];
			}
		return result;

	}
	/// convert flags to string
	std::string get_status_string(VRStatus status)
	{
		switch (status) {
		case VRS_DETACHED: return "detached";
		case VRS_ATTACHED: return "attached";
		case VRS_TRACKED: return "tracked";
		default: return "unknown status";
		}
	}
}