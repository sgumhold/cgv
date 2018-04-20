#include <gamepad/gamepad.h>
#include <iostream>

using namespace gamepad;

int main(int argc, char** argv)
{
	// enumerate drivers
	const std::vector<driver_info>& driver_infos = get_driver_infos();
	if (driver_infos.empty()) {
		std::cout << "no gamepad drivers registered, try to run under windows" << std::endl;
		return -1;
	}
	for (auto& driver_info : driver_infos) {
		std::cout << "driver '" << driver_info.name << "' " << (driver_info.enabled ? "enabled" : "disabled") << std::endl;
	}
	// scan devices
	scan_devices();
	// enumerate devices
	const std::vector<device_info>& device_infos = get_device_infos();
	if (device_infos.empty()) {
		std::cout << "no gamepad devices connected" << std::endl;
		return -2;
	}
	unsigned device_index = 0;
	for (auto& device_info : device_infos) {
		std::cout << "device '" << device_info.name << "'" 
			<< (device_info.force_feedback_support ? " ffb" : "")
			<< (device_info.is_wireless ? " wireless" : "")
			<< " vibration: " << device_info.vibration_strength[0] << "," << device_info.vibration_strength[1] << " ";
		BatteryType battery_type;
		float fill_state;
		get_device_battery_info(device_index, battery_type, fill_state);
		switch (battery_type) {
		case BT_WIRED: std::cout << "wired=" << fill_state; break;
		case BT_ALKALINE: std::cout << "alkaline=" << fill_state; break;
		case BT_NIMH: std::cout << "nimh=" << fill_state; break;
		case BT_UNKNOWN: std::cout << "unkown"; break;
		}
		std::cout << std::endl;
		++device_index;
	}
	// keep one time stamp per device
	std::vector<unsigned> last_time_stamps(device_infos.size(), 0);
	gamepad_key_event gpk;
	gamepad_state state;
	while (true) {
		// loop all devices
		for (unsigned device_index = 0; device_index < device_infos.size(); ++device_index) {
			// if state query unsuccessful rescan devices and stop this for loop
			if (!get_state(device_index, state)) {
				scan_devices();
				last_time_stamps.resize(device_infos.size());
				break;
			}
			// only show state in case of time stamp change
			if (state.time_stamp != last_time_stamps[device_index]) {
				last_time_stamps[device_index] = state.time_stamp;
				std::cout << "state[" << device_index << "] LS("
					<< state.left_stick_position[0] << "x" << state.left_stick_position[1] << ") RS("
					<< state.right_stick_position[0] << "x" << state.right_stick_position[1] << ") T("
					<< state.trigger_position[0] << "|" << state.trigger_position[1] << ")";
				if (state.button_flags > 0)
					std::cout << " flags=" << convert_flags_to_string(GamepadButtonStateFlags(state.button_flags));
				std::cout << std::endl;

				set_vibration(device_index, state.trigger_position[0], state.trigger_position[1]);
			}

			if (query_key_event(device_index, gpk)) {
				std::cout << "device[" << device_index << "]: key ";
				switch (gpk.action) {
				case GPA_UNKNOWN: std::cout << "unkown " << std::endl; break;
				case GPA_RELEASE: std::cout << "release" << std::endl; break;
				case GPA_PRESS: std::cout << "press  " << std::endl; break;
				case GPA_REPEAT: std::cout << "repeat " << std::endl; break;
				}
				std::cout << " " << convert_key_to_string(gpk.key) << std::endl;

				if (gpk.action == GPA_PRESS && gpk.key == GPK_X)
					return 1;
			}
		}
	}
}
