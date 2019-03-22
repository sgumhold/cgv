#pragma once

#include <string>
#include <vector>

#include "lib_begin.h"

///@ingroup VR
///@{

/**@file 
  defines types to store the state vr::vr_kit_state of a vr kit, which is split into 
  sub states for the hmd (vr::vr_trackable_state) and the two 
  controllers (vr::vr_conroller_state).
*/
///
namespace vr {
	/// repeated definition from cgv/gui/key_event.h
	enum KeyAction {
		KA_RELEASE, 
		KA_PRESS, 
		KA_REPEAT 
	};
	/// enumerate all VR keys starting at 1024
	enum VRKeys {
		VR_UNKNOWN = 1024,
		VR_LEFT_MENU,           //!< <em>left controller</em> VIVE: menu button; occulus: start button
		VR_LEFT_BUTTON0,        //!< <em>left controller</em> VIVE: grip button; occulus: X button
		VR_LEFT_BUTTON1,        //!< <em>left controller</em> VIVE: not used; occulus: Y button 
		VR_LEFT_BUTTON2,        //!< <em>left controller</em> VIVE: not used; occulus: not used
		VR_LEFT_BUTTON3,        //!< <em>left controller</em> not used
		VR_LEFT_STICK_TOUCH,    //!< <em>left controller</em> VIVE: touch sensor; occulus: not used
		VR_LEFT_STICK_DOWNLEFT, //!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): down left
		VR_LEFT_STICK_DOWN,     //!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): down 
		VR_LEFT_STICK_DOWNRIGHT,//!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): down right
		VR_LEFT_STICK_LEFT,     //!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): left
		VR_LEFT_STICK,          //!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): neutral
		VR_LEFT_STICK_RIGHT,    //!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): right
		VR_LEFT_STICK_UPLEFT,   //!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): up left
		VR_LEFT_STICK_UP,       //!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): up
		VR_LEFT_STICK_UPRIGHT,  //!< <em>left controller</em> press key events (VIVE-touch pad|occulus stick): up right
		VR_RIGHT_MENU,           //!< <em>right controller</em> VIVE: menu button; occulus: start button
		VR_RIGHT_BUTTON0,		 //!< <em>right controller</em> VIVE: grip button; occulus: X button
		VR_RIGHT_BUTTON1,		 //!< <em>right controller</em> VIVE: not used; occulus: Y button 
		VR_RIGHT_BUTTON2,		 //!< <em>right controller</em> VIVE: not used; occulus: not used
		VR_RIGHT_BUTTON3,		 //!< <em>right controller</em> not used
		VR_RIGHT_STICK_TOUCH,	 //!< <em>right controller</em> VIVE: touch sensor; occulus: not used
		VR_RIGHT_STICK_DOWNLEFT,  //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick): down left
		VR_RIGHT_STICK_DOWN,	  //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick):down 
		VR_RIGHT_STICK_DOWNRIGHT, //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick):down right
		VR_RIGHT_STICK_LEFT,	  //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick):left
		VR_RIGHT_STICK,			  //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick):neutral
		VR_RIGHT_STICK_RIGHT,	  //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick):right
		VR_RIGHT_STICK_UPLEFT,	  //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick):up left
		VR_RIGHT_STICK_UP,		  //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick):up
		VR_RIGHT_STICK_UPRIGHT,	  //!< <em>right controller</em> press key events (VIVE-touch pad|occulus stick):up right
		VR_END, //!< end marker used in for loops over all VR keys
		VR_BEGIN = VR_LEFT_MENU //!< begin marker used in for loops over all VR keys
	};

	/// one flag for each vr controler button
	enum VRButtonStateFlags
	{
		VRF_MENU        = 0x0001, //!< flag for menu button
		VRF_BUTTON0     = 0x0002, //!< flag for button 0
		VRF_BUTTON1     = 0x0004, //!< flag for button 1
		VRF_BUTTON2     = 0x0008, //!< flag for button 2
		VRF_BUTTON3     = 0x0010, //!< flag for button 3
		VRF_STICK_TOUCH = 0x0020, //!< flag for VIVE:touchpad|occulus:stick touch
		VRF_STICK       = 0x0040  //!< flag for VIVE:touchpad|occulus:stick press 
	};

	/// different status values for a trackable
	enum VRStatus
	{
		VRS_DETACHED, //!< trackable is not reachable via wireless
		VRS_ATTACHED, //!< trackable is connected via wireless but not tracked
		VRS_TRACKED   //!< trackable is connected and tracked
	};
	//! a trackable knows whether it is tracked and its 6d pose stored as 3x4 matrix in column major format
	/*! provides vr_trackable_state::status as vr::VRStatus and vr_trackable_state::pose of a trackable sub device
		where the pose is stored in a 3x4 column major format */
	struct CGV_API vr_trackable_state
	{
		/// whether trackable is currently tracked, only in case of true, the pose member contains useful information
		VRStatus status;
		//! pose as 3x4 matrix in column major format, where each column is a vector in world coordinates
		/*!
		- pose[0..2]  ... x-axis pointing to the right
		- pose[3..5]  ... y-axis pointing up
		- pose[6..8]  ... z-axis pointing backwards
		- pose[9..11] ... location of trackable's origin
		*/
		float pose[12];
		/// equality check
		bool operator == (const vr_trackable_state& state) const;
		/// standard constructor for initialization of members
		vr_trackable_state();
	};
	//! Extends the trackable state by information on the buttons, input axes and vibration strengths
	/*! extends the trackable state by a vr_controller_state::time_stamp, vr_controller_state::button_flags, 
	vr_controller_state::axes[8], and vr_controller_state::vibration[2]
	There are 7 buttons as listed in vr::VRButtonStateFlags, where vr::VRF_STICK_TOUCH
	corresponds to touching of the stick|touchpad and vr::VRF_STICK to pressing it.The
	controller provides up to 8 axes, where the first two correspond to the stick|touchpad
	position and the third (vr_controller_state::axes[2]) to the trigger.*/
	struct CGV_API vr_controller_state : public vr_trackable_state
	{
		/// a unique time stamp for fast test whether state changed
		unsigned time_stamp;
		/// combination of flags in VRButtonStateFlags combined with the OR operation
		unsigned button_flags;
		/// up to 8 axis values in the range [-1,1] or [0,1] (VIVE: 0|1..touchpad x|y [-1,1], 2..trigger [0,1])
		float axes[8];
		/// strength of the vibration motors
		float vibration[2];
		/// equal comparison operator
		bool operator == (const vr_controller_state& state) const;
		/// standard constructor for initialization of members
		vr_controller_state();
		/// place the 3d ray origin and the 3d ray direction into the given arrays which must provide space for 3 floats each
		void put_ray(float* ray_origin, float* ray_direction) const;
	};
	//! structure that stores all information describing the state of a VR kit
	/*! simply combines state information on the vr_kit_state::hmd (vr::vr_trackable_state) and the two controllers
		vr_kit_state::controller[2] (vr::vr_controller_state).The controllers are always enumerated such that
		controller index 0 is the left and 1 the right controller.*/
	struct CGV_API vr_kit_state
	{
		/// status and pose of hmd
		vr_trackable_state hmd;
		/// status, pose, button, axes, and vibration information of controlers
		vr_controller_state controller[2];
		/// check for equality
		bool operator == (const vr_kit_state& state) const;
		/// standard constructor for initialization of members
		vr_kit_state();
	};
	/// convert key to string
	extern CGV_API std::string get_key_string(unsigned short key);
	/// convert flags to string
	extern CGV_API std::string get_state_flag_string(VRButtonStateFlags flags);
	/// convert flags to string
	extern CGV_API std::string get_status_string(VRStatus status);
}
///@}

#include <cgv/config/lib_end.h>
