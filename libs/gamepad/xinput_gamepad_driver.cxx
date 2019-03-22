#ifdef WIN32

#include <cassert>
#include "gamepad_driver.h"
#include <Windows.h>
#include <Xinput.h>

#ifdef WIN32
#pragma warning (disable:4995)
#endif

using namespace gamepad;

///
float convert(BYTE value) { return (float)value / 255; }
float convert(SHORT value) { return value > 0 ? (float)value / 32767 : (float)value / 32768; }
float convert(WORD value) { return (float)value / 65535; }

// encode user index in handle
void* get_handle(int user_index)
{
	void* device_handle = 0;
	(int&)device_handle = user_index;
	return device_handle;
}

// decode user index from handle
int get_user_index(void* device_handle)
{
	int user_index = (int&)device_handle;
	assert(user_index >= 0 && user_index < 4);
	return user_index;
}

struct xinput_gamepad_driver : public gamepad_driver
{
	bool enabled[4];
	/// construct driver
	xinput_gamepad_driver(const std::string& options)
	{
		for (unsigned user_index = 0; user_index < 4; ++user_index)
			enabled[user_index] = true;
	}
	/// return name of driver
	std::string get_name()
	{
		return "XInput Gamepad Driver";
	}
	/// set the state to enabled or disabled
	void set_driver_state(bool enabled)
	{
		XInputEnable(enabled ? TRUE : FALSE);
	}
	/// scan all connected devices found by driver
	void scan_devices(std::vector<device_info>& infos)
	{
		XINPUT_CAPABILITIES caps;
		unsigned nr_devices = 0;
		for (unsigned user_index = 0; user_index < 4; ++user_index) {
			DWORD result = XInputGetCapabilities(user_index, XINPUT_FLAG_GAMEPAD, &caps);
			if (result == ERROR_DEVICE_NOT_CONNECTED) {
				enabled[user_index] = false;
				continue;
			}
			enabled[user_index] = true;
			infos.resize(infos.size() + 1);
			infos.back().device_handle = get_handle(user_index);
			infos.back().enabled = true;
			infos.back().force_feedback_support = (caps.Flags | XINPUT_CAPS_FFB_SUPPORTED) != 0;
			infos.back().is_wireless = (caps.Flags | XINPUT_CAPS_WIRELESS) != 0;
			infos.back().no_menu_buttons = (caps.Flags | XINPUT_CAPS_NO_NAVIGATION) != 0;
			infos.back().name = std::string("controler_");
			infos.back().name += '0' + user_index;
			infos.back().vibration_strength[0] = convert(caps.Vibration.wLeftMotorSpeed);
			infos.back().vibration_strength[1] = convert(caps.Vibration.wRightMotorSpeed);
		}
	}
	/// set the state of a device to enabled or disabled
	void set_device_state(void* device_handle, bool _enabled)
	{
		enabled[get_user_index(device_handle)] = _enabled;
	}
	/// return the battery type and state of a device, fill_state in [0,1] is only given for alkaline or nickel metal hydide batteries
	bool get_device_battery_info(void* device_handle, BatteryType& battery_type, float& fill_state)
	{
		int user_index = get_user_index(device_handle);
		XINPUT_BATTERY_INFORMATION battery_info;
		DWORD result = XInputGetBatteryInformation(user_index, BATTERY_DEVTYPE_GAMEPAD, &battery_info);
		if (result == ERROR_DEVICE_NOT_CONNECTED)
			return false;
		switch (battery_info.BatteryType) {
		case BATTERY_TYPE_DISCONNECTED: return false;
		case BATTERY_TYPE_WIRED:    battery_type = BT_WIRED; break;
		case BATTERY_TYPE_ALKALINE: battery_type = BT_ALKALINE; break;
		case BATTERY_TYPE_NIMH:	battery_type = BT_NIMH; break;
		case BATTERY_TYPE_UNKNOWN:	battery_type = BT_UNKNOWN; break;
		}

		switch (battery_info.BatteryLevel) {
		case BATTERY_LEVEL_EMPTY: fill_state = 0.0f; break;
		case BATTERY_LEVEL_LOW: fill_state = 0.2f; break;
		case BATTERY_LEVEL_MEDIUM: fill_state = 0.5f; break;
		case BATTERY_LEVEL_FULL: fill_state = 1.0f; break;
		}
		return true;
	}
	/// query event queue of given device for single gamepad key event
	bool query_device_key_event(void* device_handle, GamepadKeys& gk, KeyAction& action)
	{
		XINPUT_KEYSTROKE keystroke;
		DWORD result = XInputGetKeystroke(get_user_index(device_handle), 0, &keystroke);
		if (result == ERROR_DEVICE_NOT_CONNECTED)
			return false;
		if (result == ERROR_EMPTY)
			return false;
		switch (keystroke.VirtualKey) {
		case VK_PAD_A:                 gk = GPK_A;				 break;
		case VK_PAD_B:				   gk = GPK_B;				 break;
		case VK_PAD_X:				   gk = GPK_X;				 break;
		case VK_PAD_Y:				   gk = GPK_Y;				 break;
		case VK_PAD_RSHOULDER:		   gk = GPK_RIGHT_BUMPER;		 break;
		case VK_PAD_LSHOULDER:		   gk = GPK_LEFT_BUMPER;		 break;
		case VK_PAD_LTRIGGER:		   gk = GPK_LEFT_TRIGGER;			 break;
		case VK_PAD_RTRIGGER:		   gk = GPK_RIGHT_TRIGGER;			 break;
		case VK_PAD_DPAD_UP:		   gk = GPK_DPAD_UP;			 break;
		case VK_PAD_DPAD_DOWN:		   gk = GPK_DPAD_DOWN;		 break;
		case VK_PAD_DPAD_LEFT:		   gk = GPK_DPAD_LEFT;		 break;
		case VK_PAD_DPAD_RIGHT:		   gk = GPK_DPAD_RIGHT;		 break;
		case VK_PAD_START:			   gk = GPK_START;			 break;
		case VK_PAD_BACK:			   gk = GPK_BACK;				 break;
		case VK_PAD_LTHUMB_PRESS:	   gk = GPK_LEFT_STICK_PRESS;		 break;
		case VK_PAD_RTHUMB_PRESS:	   gk = GPK_RIGHT_STICK_PRESS;		 break;
		case VK_PAD_LTHUMB_UP:		   gk = GPK_LEFT_STICK_UP;		 break;
		case VK_PAD_LTHUMB_DOWN:	   gk = GPK_LEFT_STICK_DOWN;		 break;
		case VK_PAD_LTHUMB_RIGHT:	   gk = GPK_LEFT_STICK_RIGHT;		 break;
		case VK_PAD_LTHUMB_LEFT:	   gk = GPK_LEFT_STICK_LEFT;		 break;
		case VK_PAD_LTHUMB_UPLEFT:	   gk = GPK_LEFT_STICK_UPLEFT;	 break;
		case VK_PAD_LTHUMB_UPRIGHT:	   gk = GPK_LEFT_STICK_UPRIGHT;	 break;
		case VK_PAD_LTHUMB_DOWNRIGHT:  gk = GPK_LEFT_STICK_DOWNRIGHT;	 break;
		case VK_PAD_LTHUMB_DOWNLEFT:   gk = GPK_LEFT_STICK_DOWNLEFT;	 break;
		case VK_PAD_RTHUMB_UP:		   gk = GPK_RIGHT_STICK_UP;		 break;
		case VK_PAD_RTHUMB_DOWN:	   gk = GPK_RIGHT_STICK_DOWN;		 break;
		case VK_PAD_RTHUMB_RIGHT:	   gk = GPK_RIGHT_STICK_RIGHT;		 break;
		case VK_PAD_RTHUMB_LEFT:	   gk = GPK_RIGHT_STICK_LEFT;		 break;
		case VK_PAD_RTHUMB_UPLEFT:	   gk = GPK_RIGHT_STICK_UPLEFT;	 break;
		case VK_PAD_RTHUMB_UPRIGHT:	   gk = GPK_RIGHT_STICK_UPRIGHT;	 break;
		case VK_PAD_RTHUMB_DOWNRIGHT:  gk = GPK_RIGHT_STICK_DOWNRIGHT;	 break;
		case VK_PAD_RTHUMB_DOWNLEFT:   gk = GPK_RIGHT_STICK_DOWNLEFT;   break;
		default:                       gk = GPK_UNKNOWN; break;
		}
		switch (keystroke.Flags) {
		case XINPUT_KEYSTROKE_KEYUP: action = KA_RELEASE; break;
		case XINPUT_KEYSTROKE_KEYDOWN: action = KA_PRESS; break;
		case XINPUT_KEYSTROKE_KEYDOWN + XINPUT_KEYSTROKE_REPEAT: action = KA_REPEAT; break;
		default: action = KeyAction(-1); break;
		}
		return true;
	}
	/// retrieve the current state of gamepad stick and trigger positions
	bool xinput_gamepad_driver::get_device_state(void* device_handle, gamepad_state& state)
	{
		XINPUT_STATE pad_state;
		DWORD result = XInputGetState(get_user_index(device_handle), &pad_state);
		if (result == ERROR_DEVICE_NOT_CONNECTED)
			return false;
		state.time_stamp = pad_state.dwPacketNumber;
		state.button_flags = pad_state.Gamepad.wButtons;
		state.trigger_position[0] = convert(pad_state.Gamepad.bLeftTrigger);
		state.trigger_position[1] = convert(pad_state.Gamepad.bRightTrigger);
		state.left_stick_position[0] = convert(pad_state.Gamepad.sThumbLX);
		state.left_stick_position[1] = convert(pad_state.Gamepad.sThumbLY);
		state.right_stick_position[0] = convert(pad_state.Gamepad.sThumbRX);
		state.right_stick_position[1] = convert(pad_state.Gamepad.sThumbRY);
		return true;
	}
	/// set the vibration strength between 0 and 1 of left and right motor
	bool xinput_gamepad_driver::set_device_vibration(void* device_handle, float low_frequency_strength, float high_frequency_strength)
	{
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = WORD(low_frequency_strength * 65535);
		vibration.wRightMotorSpeed = WORD(high_frequency_strength * 65535);
		DWORD result = XInputSetState(get_user_index(device_handle), &vibration);
		if (result == ERROR_DEVICE_NOT_CONNECTED)
			return false;
		return true;
	}
};

driver_registry<xinput_gamepad_driver> xinput_gpd_registry("xinput");

#endif