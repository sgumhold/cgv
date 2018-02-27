#pragma once

#include <string>
#include <vector>

#include "lib_begin.h"

namespace gamepad {

	/**@name gamepad driver management*/
	//@{
	/// information structure for drivers
	struct driver_info
	{
		/// name of driver
		std::string name;
		/// state of driver
		bool enabled;
	};
	/// return information on the registered drivers
	extern CGV_API const std::vector<driver_info>& get_driver_infos();
	/// set the state of a driver to enabled or disabled
	extern CGV_API void set_driver_state(unsigned i, bool enabled);
	//@}

	/**@name gamepad device management */
	//@{
	/// information provided per gamepad device
	struct device_info
	{
		/// name in case driver provides this information (not reliable)
		std::string name;
		/// whether force feedback is supported
		bool force_feedback_support;
		/// whether it is wireless
		bool is_wireless;
		/// whether menu buttons (start,back) are missing
		bool no_menu_buttons;
		/// index of driver used to access device
		size_t driver_index;
		/// whether device is enabled
		bool enabled;
		/// strength values of low and high frequency vibration in the range [0,1]
		float vibration_strength[2];
	};

	/// scan all drivers for connected devices
	extern CGV_API void scan_devices();
	/// return reference to device info structures
	extern CGV_API const std::vector<device_info>& get_device_infos();
	/// check if device is still connected
	extern CGV_API bool is_connected(unsigned device_index);
	/// set the state of a device to enabled or disabled, return false in case device was not connected anymore
	extern CGV_API bool set_device_state(unsigned device_index, bool enabled);
	
	/// different battery types
	enum BatteryType {
		BT_WIRED,
		BT_ALKALINE,
		BT_NIMH, // nickel metal hydide batteries
		BT_UNKNOWN
	};
	/// retrieve battery type and in case of alkaline or nickel metal hydide batteries als fill state (in range 0..1) of given device, return false in case device was not connected anymore
	extern CGV_API bool get_device_battery_info(unsigned device_index, BatteryType& battery_type, float& fill_state);
	//@}

	/**@name gamepad device usage */
	//@{
	/// see https://upload.wikimedia.org/wikipedia/commons/2/2c/360_controller.svg for an explanation of the keys
	enum GamepadKeys {
		GPK_UNKNOWN = 512,

		GPK_A,
		GPK_B,
		GPK_X,
		GPK_Y,
		GPK_RIGHT_BUMPER,
		GPK_LEFT_BUMPER,
		GPK_LEFT_TRIGGER,
		GPK_RIGHT_TRIGGER,
		GPK_DPAD_UP,
		GPK_DPAD_DOWN,
		GPK_DPAD_LEFT,
		GPK_DPAD_RIGHT,
		GPK_START,
		GPK_BACK,

		GPK_LEFT_STICK_PRESS,
		GPK_RIGHT_STICK_PRESS,
		GPK_LEFT_STICK_UP,
		GPK_LEFT_STICK_DOWN,
		GPK_LEFT_STICK_RIGHT,
		GPK_LEFT_STICK_LEFT,
		GPK_LEFT_STICK_UPLEFT,
		GPK_LEFT_STICK_UPRIGHT,
		GPK_LEFT_STICK_DOWNRIGHT,
		GPK_LEFT_STICK_DOWNLEFT,

		GPK_RIGHT_STICK_UP,
		GPK_RIGHT_STICK_DOWN,
		GPK_RIGHT_STICK_RIGHT,
		GPK_RIGHT_STICK_LEFT,
		GPK_RIGHT_STICK_UPLEFT,
		GPK_RIGHT_STICK_UPRIGHT,
		GPK_RIGHT_STICK_DOWNRIGHT,
		GPK_RIGHT_STICK_DOWNLEFT,
		GPK_END,
		GPK_BEGIN = GPK_A
	};
	/// convert key to string
	extern CGV_API std::string convert_key_to_string(unsigned short key);
	/// different key actions
	enum GamepadAction {
		GPA_UNKNOWN,
		GPA_RELEASE,
		GPA_PRESS,
		GPA_REPEAT
	};

	/// gamepad key event
	struct gamepad_key_event
	{
		/// one out of GamepadKey enum
		unsigned short key;
		/// one out of GamepadAction
		unsigned short action;
	};

	//! retrieve next key event from given device, return false if device's event queue is empty
	/*!Typically you use this function in the following way:
		while (query_key_event(i,gke)) { process(gke); }
	*/
	extern CGV_API bool query_key_event(unsigned device_index, gamepad_key_event& gke);
	
	/// one flag for for each gamepad button
	enum GamepadButtonStateFlags
	{
		GBF_DPAD_UP	     = 0x0001,
		GBF_DPAD_DOWN    = 0x0002,
		GBF_DPAD_LEFT    = 0x0004,
		GBF_DPAD_RIGHT   = 0x0008,
		GBF_START	     = 0x0010,
		GBF_BACK	     = 0x0020,
		GBF_LEFT_STICK	 = 0x0040,
		GBF_RIGHT_STICK	 = 0x0080,
		GBF_LEFT_BUMPER	 = 0x0100,
		GBF_RIGHT_BUMPER = 0x0200,
		GBF_A            = 0x1000,
		GBF_B            = 0x2000,
		GBF_X            = 0x4000,
		GBF_Y            = 0x8000				 
	};
	/// convert flags to string
	extern CGV_API std::string convert_flags_to_string(GamepadButtonStateFlags flags);

	/// see https://upload.wikimedia.org/wikipedia/commons/2/2c/360_controller.svg for an explanation of the positions in the state
	struct gamepad_state
	{
		/// time stamp can be used whether a change has happened between two states
		unsigned time_stamp;
		/// combination of flags in GamepadButtonStateFlags combined with the OR operation
		unsigned button_flags;
		/// x and y position of left thumb in the range [-1,1]
		float left_stick_position[2];
		/// x and y position of left thumb in the range [-1,1]
		float right_stick_position[2];
		/// values of left and right triggers in the range [0,1]
		float trigger_position[2];
	};

	/// retrieve the current state of gamepad stick and trigger positions, return false if device is not connected anymore
	extern CGV_API bool get_state(unsigned device_index, gamepad_state& state);
	/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
	extern CGV_API bool set_vibration(unsigned device_index, float low_frequency_strength, float high_frequency_strength);
}

#include <cgv/config/lib_end.h>
