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
	/// maximum number of attachable controller and tracker devices
	const unsigned max_nr_controllers = 8;
	/// maximum number of inputs per controller
	const unsigned max_nr_controller_inputs = 5;
	/// maximum number of axes per controller
	const unsigned max_nr_controller_axes = 8;

	/// repeated definition from cgv/gui/key_event.h
	enum KeyAction {
		KA_RELEASE, 
		KA_PRESS, 
		KA_REPEAT 
	};
	/// enumerate all VR keys starting at 1024
	enum VRKeys {
		VR_UNKNOWN = 1024,
		VR_SYSTEM,          //!< VIVE: system button; occulus: ???
		VR_MENU,            //!< VIVE: menu button; occulus: start button
		VR_GRIP,            //!< grip button
		VR_DPAD_DOWN_LEFT,  //!< direction pad diagonally down and left
		VR_DPAD_DOWN,       //!< direction pad down
		VR_DPAD_DOWN_RIGHT, //!< direction pad diagonally down and right
		VR_DPAD_LEFT,       //!< direction pad left
		VR_DPAD_RIGHT,      //!< direction pad right
		VR_DPAD_UP_LEFT,    //!< direction pad diagonally up and left
		VR_DPAD_UP,         //!< direction pad up
		VR_DPAD_UP_RIGHT,   //!< direction pad diagonally up and right
		VR_A,               //!< A button
		VR_INPUT0_TOUCH,    //!< touched input 0
		VR_INPUT0,          //!< input 0
		VR_INPUT1_TOUCH,    //!< touched input 1
		VR_INPUT1,          //!< input 1
		VR_INPUT2_TOUCH,    //!< touched input 2
		VR_INPUT2,          //!< input 2
		VR_INPUT3_TOUCH,    //!< touched input 3
		VR_INPUT3,          //!< input 3
		VR_INPUT4_TOUCH,    //!< touched input 4
		VR_INPUT4,          //!< input 4
		VR_PROXIMITY,       //!< proximity sensor
		VR_END,             //!< end marker used in for loops over all VR keys
		VR_BEGIN=VR_SYSTEM  //!< begin marker used in for loops over all VR keys
	};
	/// one flag for each vr controller button
	enum VRButtonStateFlags
	{
		VRF_SYSTEM       = 0x000001, //!< system button
		VRF_MENU         = 0x000002, //!< application menu button
		VRF_GRIP         = 0x000004, //!< grip button
		VRF_DPAD_LEFT    = 0x000008, //!< direction pad left button
		VRF_DPAD_RIGHT   = 0x000010, //!< direction pad right button
		VRF_DPAD_DOWN    = 0x000020, //!< direction pad down button
		VRF_DPAD_UP      = 0x000040, //!< direction pad up button
		VRF_A            = 0x000080, //!< A button
		VRF_INPUT0_TOUCH = 0x000100, //!< touch sensor for input 0 which often is touchpad or stick
		VRF_INPUT0       = 0x000200, //!< button of input 0 
		VRF_INPUT1_TOUCH = 0x000400, //!< touch sensor for input 1 which often is touchpad or stick
		VRF_INPUT1       = 0x000800, //!< button of input 1
		VRF_INPUT2_TOUCH = 0x001000, //!< touch sensor for input 2 which often is touchpad or stick
		VRF_INPUT2       = 0x002000, //!< button of input 2
		VRF_INPUT3_TOUCH = 0x004000, //!< touch sensor for input 3 which often is touchpad or stick
		VRF_INPUT3       = 0x008000, //!< button of input 3
		VRF_INPUT4_TOUCH = 0x010000, //!< touch sensor for input 4 which often is touchpad or stick
		VRF_INPUT4       = 0x020000, //!< button of input 4
		VRF_PROXIMITY    = 0x040000  //!< proximity sensor
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
	vr_controller_state::axes[vr::max_nr_controller_axes], and vr_controller_state::vibration[2]
	There are 7 buttons as listed in vr::VRButtonStateFlags, where vr::VRF_STICK_TOUCH
	corresponds to touching of the stick|touchpad and vr::VRF_STICK to pressing it.The
	controller provides up to \c vr::max_nr_controller_axes axes, where the first two correspond to the stick|touchpad
	position and the third (vr_controller_state::axes[2]) to the trigger.*/
	struct CGV_API vr_controller_state : public vr_trackable_state
	{
		/// a unique time stamp for fast test whether state changed
		unsigned time_stamp;
		/// combination of flags in VRButtonStateFlags combined with the OR operation
		unsigned button_flags;
		/// up to \c vr::max_nr_controller_axes axis values in the range [-1,1] or [0,1] (VIVE: 0|1..touchpad x|y [-1,1], 2..trigger [0,1])
		float axes[max_nr_controller_axes];
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
		/// status, pose, button, axes, and vibration information of up to \c vr::max_nr_controllers controller and tracker devices (first two are for left and right hand controllers)
		vr_controller_state controller[max_nr_controllers];
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
