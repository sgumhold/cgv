#pragma once

#include <string>
#include <vector>

#include "lib_begin.h"

namespace vr {
	/// repeated definition from cgv/gui/key_event.h
	enum KeyAction {
		KA_RELEASE, //!< key release action
		KA_PRESS, //!< key press action
		KA_REPEAT //!< key repeated press action
	};
	/// enumerate all VR keys starting at 1024
	enum VRKeys {
		VR_UNKNOWN = 1024,

		VR_LEFT_MENU,
		VR_LEFT_BUTTON0,
		VR_LEFT_BUTTON1,
		VR_LEFT_BUTTON2,
		VR_LEFT_BUTTON3,
		VR_LEFT_TRIGGER,

		VR_LEFT_STICK_PRESS,
		VR_LEFT_STICK_UP,
		VR_LEFT_STICK_DOWN,
		VR_LEFT_STICK_RIGHT,
		VR_LEFT_STICK_LEFT,
		VR_LEFT_STICK_UPLEFT,
		VR_LEFT_STICK_UPRIGHT,
		VR_LEFT_STICK_DOWNRIGHT,
		VR_LEFT_STICK_DOWNLEFT,

		VR_RIGHT_MENU,
		VR_RIGHT_BUTTON0,
		VR_RIGHT_BUTTON1,
		VR_RIGHT_BUTTON2,
		VR_RIGHT_BUTTON3,
		VR_RIGHT_TRIGGER,

		VR_RIGHT_STICK_PRESS,
		VR_RIGHT_STICK_UP,
		VR_RIGHT_STICK_DOWN,
		VR_RIGHT_STICK_RIGHT,
		VR_RIGHT_STICK_LEFT,
		VR_RIGHT_STICK_UPLEFT,
		VR_RIGHT_STICK_UPRIGHT,
		VR_RIGHT_STICK_DOWNRIGHT,
		VR_RIGHT_STICK_DOWNLEFT,

		VR_END,
		VR_BEGIN = VR_LEFT_MENU
	};
	/// one flag for each vr controler button
	enum VRButtonStateFlags
	{
		VRF_MENU     = 0x0001,
		VRF_BUTTON0  = 0x0002,
		VRF_BUTTON1  = 0x0004,
		VRF_BUTTON2  = 0x0008,
		VRF_BUTTON3  = 0x0010,
		VRF_TOUCH    = 0x0020,
		VRF_PRESS    = 0x0040
	};
	/// different status values for a trackable
	enum VRStatus
	{
		VRS_detached,
		VRS_attached,
		VRS_tracked
	};
	/// a trackable knows whether it is tracked and its 6d pose stored as 3x4 matrix in column major format
	struct vr_trackable_state
	{
		/// whether trackable is currently tracked, only in case of true, the pose member contains useful information
		VRStatus status;
		//! pose as 3x4 matrix in column major format, where each column is a vector in world coordinates
		/*!  - pose[0..2]  ... x-axis pointing to the right
			 - pose[3..5]  ... y-axis pointing up
			 - pose[6..8]  ... z-axis pointing backwards
			 - pose[9..11] ... location of trackable's origin */
		float pose[12];
		///
		bool operator == (const vr_trackable_state& state) const;
	};
	/// the controller state extends the trackable state by information on the buttons, input axes and vibration strengths
	struct vr_controller_state : public vr_trackable_state
	{
		/// a unique time stamp for fast test whether state changed
		unsigned time_stamp;
		/// combination of flags in VRButtonStateFlags combined with the OR operation
		unsigned button_flags;
		/// up to 8 axis values in the range [-1,1] or [0,1] (VIVE: 0|1..touchpad x|y [-1,1], 2..trigger [0,1])
		float axes[8];
		/// strength of the vibration motors
		float vibration[2];
		///
		bool operator == (const vr_controller_state& state) const;
	};
	/// structure that stores all information describing the state of the VR kit
	struct vr_kit_state
	{
		/// status and pose of hmd
		vr_trackable_state hmd;
		/// status, pose, button, axes, and vibration information of controlers
		vr_controller_state controller[2];
		/// check for equality
		bool operator == (const vr_kit_state& state) const;
	};
	/// convert key to string
	extern CGV_API std::string get_key_string(unsigned short key);
	/// convert flags to string
	extern CGV_API std::string get_state_flag_string(VRButtonStateFlags flags);
	/// convert flags to string
	extern CGV_API std::string get_status_string(VRStatus status);
}

#include <cgv/config/lib_end.h>
