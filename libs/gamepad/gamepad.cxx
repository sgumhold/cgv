#include "gamepad.h"
#include "gamepad_driver.h"
#include <cassert>

namespace gamepad {

	std::string get_key_string(unsigned short key)
	{
		static const char* gamepad_key_names[] = {
			"UNKNOWN",

			"A",
			"B",
			"X",
			"Y",

			"RIGHT_BUMPER",
			"LEFT_BUMPER",
			"LEFT_TRIGGER",
			"RIGHT_TRIGGER",

			"DPAD_UP",
			"DPAD_DOWN",
			"DPAD_LEFT",
			"DPAD_RIGHT",

			"START",
			"BACK",

			"LEFT_STICK_PRESS",
			"RIGHT_STICK_PRESS",

			"LEFT_STICK_UP",
			"LEFT_STICK_DOWN",
			"LEFT_STICK_RIGHT",
			"LEFT_STICK_LEFT",
			"LEFT_STICK_UPLEFT",
			"LEFT_STICK_UPRIGHT",
			"LEFT_STICK_DOWNRIGHT",
			"LEFT_STICK_DOWNLEFT",

			"RIGHT_STICK_UP",
			"RIGHT_STICK_DOWN",
			"RIGHT_STICK_RIGHT",
			"RIGHT_STICK_LEFT",
			"RIGHT_STICK_UPLEFT",
			"RIGHT_STICK_UPRIGHT",
			"RIGHT_STICK_DOWNRIGHT",
			"RIGHT_STICK_DOWNLEFT"
		};
		int index = key - (int)GPK_UNKNOWN;
		return index < 33 ? gamepad_key_names[index] : "";
	}

	/// convert flags to string
	std::string convert_flags_to_string(GamepadButtonStateFlags flags)
	{
		static const char* flag_names[] = {
			"DPAD_UP"     ,
			"DPAD_DOWN"   ,
			"DPAD_LEFT"   ,
			"DPAD_RIGHT"  ,
			"START"       ,
			"BACK"        ,
			"LEFT_STICK"  ,
			"RIGHT_STICK" ,
			"LEFT_BUMPER" ,
			"RIGHT_BUMPER",
			"A",
			"B",
			"X",
			"Y"
		};
		static const GamepadButtonStateFlags flag_values[] = {
		GBF_DPAD_UP      ,
		GBF_DPAD_DOWN	 ,
		GBF_DPAD_LEFT	 ,
		GBF_DPAD_RIGHT	 ,
		GBF_START		 ,
		GBF_BACK		 ,
		GBF_LEFT_STICK	 ,
		GBF_RIGHT_STICK  ,
		GBF_LEFT_BUMPER  ,
		GBF_RIGHT_BUMPER ,
		GBF_A			 ,
		GBF_B			 ,
		GBF_X			 ,
		GBF_Y
		};
		std::string result;
		for (unsigned i = 0; i < 14; ++i)
			if ((flags & flag_values[i]) != 0) {
				if (result.empty())
					result = flag_names[i];
				else
					result += std::string("+") + flag_names[i];
			}
		return result;
	}
	/// initialize state
	gamepad_state::gamepad_state()
	{
		time_stamp = 0;
		button_flags = 0;
		left_stick_position[0] = left_stick_position[1] = 0;
		right_stick_position[0] = right_stick_position[1] = 0;
		trigger_position[0] = trigger_position[1] = 0;
	}

	/// return information on the registered drivers
	const std::vector<driver_info>& get_driver_infos()
	{
		return ref_driver_infos();
	}
	/// set the state of a driver to enabled or disabled
	void set_driver_state(unsigned i, bool enabled)
	{
		assert(i < ref_drivers().size());
		ref_drivers()[i]->set_driver_state(enabled);
		ref_driver_infos()[i].enabled = enabled;
	}

	/// scan all drivers for connected devices
	void scan_devices()
	{
		ref_device_infos().clear();
		for (auto driver_ptr : ref_drivers())
			driver_ptr->scan_devices(ref_device_infos());
	}
	/// return reference to device info structures
	const std::vector<device_info>& get_device_infos()
	{
		return ref_device_infos();
	}
	/// return pointer to device info structure of given device handle or 0 if device handle not found
	const device_info* get_device_info(void* device_handle)
	{
		return ref_device_info(device_handle);
	}

	bool get_driver_index(void* device_handle, unsigned& driver_index)
	{
		// check if device_index is valid
		auto* dip = get_device_info(device_handle);
		if (!dip)
			return false;
		driver_index = (unsigned)dip->driver_index;
		if (driver_index >= ref_drivers().size())
			return false;
		return true;
	}
	/// check if device is still connected
	bool is_connected(void* device_handle)
	{
		gamepad_state state;
		unsigned driver_index;
		if (!get_driver_index(device_handle, driver_index))
			return false;
		return ref_drivers()[driver_index]->get_device_state(device_handle, state);
	}
	/// set the state of a device to enabled or disabled, return false in case device was not connected anymore
	bool set_device_state(void* device_handle, bool enabled)
	{
		unsigned driver_index;
		if (!get_driver_index(device_handle, driver_index))
			return false;
		ref_drivers()[driver_index]->set_device_state(device_handle, enabled);
		ref_device_info(device_handle)->enabled = enabled;
		return true;
	}

	bool get_device_battery_info(void* device_handle, BatteryType& battery_type, float& fill_state)
	{
		unsigned driver_index;
		if (!get_driver_index(device_handle, driver_index))
			return false;
		return ref_drivers()[driver_index]->get_device_battery_info(device_handle, battery_type, fill_state);
	}
	bool query_key_event(void* device_handle, GamepadKeys& gk, KeyAction& action)
	{
		unsigned driver_index;
		if (!get_driver_index(device_handle, driver_index))
			return false;
		return ref_drivers()[driver_index]->query_device_key_event(device_handle, gk, action);
	}
	/// retrieve the current state of gamepad stick and trigger positions, return false if device is not connected anymore
	bool get_state(void* device_handle, gamepad_state& state)
	{
		unsigned driver_index;
		if (!get_driver_index(device_handle, driver_index))
			return false;
		return ref_drivers()[driver_index]->get_device_state(device_handle, state);
	}
	/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
	bool set_vibration(void* device_handle, float low_frequency_strength, float high_frequency_strength)
	{
		unsigned driver_index;
		if (!get_driver_index(device_handle, driver_index))
			return false;
		return ref_drivers()[driver_index]->set_device_vibration(device_handle, low_frequency_strength, high_frequency_strength);
	}
}
