#pragma once

#include "gamepad.h"

#include "lib_begin.h"

namespace gamepad {
	/**@name gamepad driver management*/
	//@{
	/// interface class for gamepad drivers, when implementing your driver, provide a constructor with a single options argument of type std::string
	struct CGV_API gamepad_driver
	{
		/// driver index is set during registration
		unsigned driver_index;
		/// return name of driver
		virtual std::string get_name() = 0;
		/// scan all connected devices found by driver
		virtual void scan_devices(std::vector<device_info>& infos) = 0;
		/// set the state to enabled or disabled
		virtual void set_driver_state(bool enabled) = 0;
		/// set the state of a device to enabled or disabled
		virtual void set_device_state(void* device_handle, bool enabled) = 0;
		/// return the battery type and state of a device, fill_state in [0,1] is only given for alkaline or nickel metal hydide batteries
		virtual bool get_device_battery_info(void* device_handle, BatteryType& battery_type, float& fill_state) = 0;
		/// query event queue of given device for single gamepad key event
		virtual bool query_device_key_event(void* device_handle, GamepadKeys& gk, KeyAction& action) = 0;
		/// retrieve the current state of gamepad stick and trigger positions
		virtual bool get_device_state(void* device_handle, gamepad_state& state) = 0;
		/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected 
		virtual bool set_device_vibration(void* device_handle, float low_frequency_strength, float high_frequency_strength) = 0;
	};
	/// return reference to device info structures
	extern CGV_API std::vector<device_info>& ref_device_infos();
	/// return pointer to device info structure of given device handle or 0 if device handle not found
	extern CGV_API device_info* ref_device_info(void* device_handle);
	/// return information on the registered drivers
	extern CGV_API std::vector<driver_info>& ref_driver_infos();
	/// return registered drivers
	extern CGV_API std::vector<gamepad_driver*>& ref_drivers();
	/// register a new driver
	extern CGV_API void register_driver(gamepad_driver* gpd);
	/// use this template to register your own driver
	template <typename T>
	struct driver_registry
	{
		driver_registry(const std::string& options)
		{
			register_driver(new T(options));
		}
	};
}

#include <cgv/config/lib_end.h>
