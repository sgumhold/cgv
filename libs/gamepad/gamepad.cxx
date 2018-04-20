#include "gamepad.h"
#include "gamepad_driver.h"
#include <cassert>

namespace gamepad {

	std::string convert_key_to_string(unsigned short key)
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
		return index < 33 ? gamepad_key_names[index] : "UNKNOWN";
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
		ref_device_handles().clear();
		for (auto driver_ptr : ref_drivers())
			driver_ptr->scan_devices(ref_device_infos(), ref_device_handles());
	}
	/// return reference to device info structures
	const std::vector<device_info>& get_device_infos()
	{
		return ref_device_infos();
	}

	bool get_driver_index(unsigned device_index, unsigned& driver_index)
	{
		// check if device_index is valid
		if (device_index >= ref_device_handles().size())
			return false;
		driver_index = ref_device_infos()[device_index].driver_index;
		if (driver_index >= ref_drivers().size())
			return false;
		return true;
	}
	/// check if device is still connected
	bool is_connected(unsigned device_index)
	{
		gamepad_state state;
		unsigned driver_index;
		if (!get_driver_index(device_index, driver_index))
			return false;
		return ref_drivers()[driver_index]->get_device_state(ref_device_handles()[device_index], state);
	}
	/// set the state of a device to enabled or disabled, return false in case device was not connected anymore
	bool set_device_state(unsigned device_index, bool enabled)
	{
		unsigned driver_index;
		if (!get_driver_index(device_index, driver_index))
			return false;
		ref_drivers()[driver_index]->set_device_state(ref_device_handles()[device_index], enabled);
		ref_device_infos()[device_index].enabled = enabled;
		return true;
	}

	bool get_device_battery_info(unsigned device_index, BatteryType& battery_type, float& fill_state)
	{
		unsigned driver_index;
		if (!get_driver_index(device_index, driver_index))
			return false;
		return ref_drivers()[driver_index]->get_device_battery_info(ref_device_handles()[device_index], battery_type, fill_state);
	}
	bool query_key_event(unsigned device_index, gamepad_key_event& gke)
	{
		unsigned driver_index;
		if (!get_driver_index(device_index, driver_index))
			return false;
		return ref_drivers()[driver_index]->query_device_key_event(ref_device_handles()[device_index], gke);
	}
	/// retrieve the current state of gamepad stick and trigger positions, return false if device is not connected anymore
	bool get_state(unsigned device_index, gamepad_state& state)
	{
		unsigned driver_index;
		if (!get_driver_index(device_index, driver_index))
			return false;
		return ref_drivers()[driver_index]->get_device_state(ref_device_handles()[device_index], state);
	}
	/// set the vibration strength between 0 and 1 of low and high frequency motors, return false if device is not connected anymore
	bool set_vibration(unsigned device_index, float low_frequency_strength, float high_frequency_strength)
	{
		unsigned driver_index;
		if (!get_driver_index(device_index, driver_index))
			return false;
		return ref_drivers()[driver_index]->set_device_vibration(ref_device_handles()[device_index], low_frequency_strength, high_frequency_strength);
	}
}


/*
#include "gamepad.h"

namespace cgv {
	namespace gui {

		const char* pad_key_names[] = {
			"A",
			"B",
			"X",
			"Y",

			"RSHOULDER",
			"LSHOULDER",
			"LTRIGGER",
			"RTRIGGER",

			"DPAD_UP",
			"DPAD_DOWN",
			"DPAD_LEFT",
			"DPAD_RIGHT",

			"START",
			"BACK",

			"LTHUMB_PRESS",
			"RTHUMB_PRESS",

			"LTHUMB_UP",
			"LTHUMB_DOWN",
			"LTHUMB_RIGHT",
			"LTHUMB_LEFT",
			"LTHUMB_UPLEFT",
			"LTHUMB_UPRIGHT",
			"LTHUMB_DOWNRIGHT",
			"LTHUMB_DOWNLEFT",

			"RTHUMB_UP",
			"RTHUMB_DOWN",
			"RTHUMB_RIGHT",
			"RTHUMB_LEFT",
			"RTHUMB_UPLEFT",
			"RTHUMB_UPRIGHT",
			"RTHUMB_DOWNRIGHT",
			"RTHUMB_DOWNLEFT"
		};

		unsigned short pad_gui_keys[]{
			'A',
			'B',
			'X',
			'Y',

			cgv::gui::KEY_F1,
			cgv::gui::KEY_F2,
			cgv::gui::KEY_F3,
			cgv::gui::KEY_F4,

			cgv::gui::KEY_Up,
			cgv::gui::KEY_Down,
			cgv::gui::KEY_Left,
			cgv::gui::KEY_Right,

			cgv::gui::KEY_Enter,
			cgv::gui::KEY_Back_Space,

			cgv::gui::KEY_Pause,
			cgv::gui::KEY_Num_Enter,

			'8',
			'2',
			'6',
			'4',
			'7',
			'9',
			'3',
			'1',

			cgv::gui::KEY_Num_8,
			cgv::gui::KEY_Num_2,
			cgv::gui::KEY_Num_6,
			cgv::gui::KEY_Num_4,
			cgv::gui::KEY_Num_7,
			cgv::gui::KEY_Num_9,
			cgv::gui::KEY_Num_3,
			cgv::gui::KEY_Num_1
		};

		/// convert a pad key code into a readable string
		std::string get_pad_key_string(unsigned short key)
		{
			if (key < 512)
				return std::string();
			if (key > (unsigned short)PAD_RTHUMB_DOWNLEFT)
				return std::string();
			return std::string(pad_key_names[key - 512]);
		}
		/// convert a pad key code to a key code
		unsigned short map_pad_key_to_key(unsigned short key)
		{
			if (key < 512)
				return key;
			if (key > (unsigned short)PAD_RTHUMB_DOWNLEFT)
				return key;
			return pad_gui_keys[key - 512]
		}
	}
}
*/