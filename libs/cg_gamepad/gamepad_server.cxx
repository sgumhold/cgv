#include "gamepad_server.h"
#include <cgv/gui/application.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/trigger.h>
#include <iomanip>

namespace cgv {
	namespace gui {
		/// construct a key event from its textual description 
		gamepad_key_event::gamepad_key_event(void* _device_id, unsigned short _key, KeyAction _action, unsigned char _char, double _time)
			: key_event(_key, _action, _char, 0, 0, _time), device_id(_device_id)
		{
			flags = EF_PAD;
		}


		/// return the device id, by default returns 0
		void* gamepad_key_event::get_device_id() const
		{
			return device_id;
		}

		/// write to stream
		void gamepad_key_event::stream_out(std::ostream& os) const
		{
			event::stream_out(os);
			os << gamepad::get_key_string(key);
			switch (action) {
			case KA_RELEASE:
				os << " up";
				break;
			case KA_PRESS:
				if (get_char())
					os << " = '" << get_char() << "'";
				break;
			case KA_REPEAT:
				os << " repeat ";
				break;
			}
			os << "[" << device_id << "]";
		}
		// read from stream
		void gamepad_key_event::stream_in(std::istream&)
		{
			std::cerr << "key_event::stream_in not implemented yet" << std::endl;
		}

		gamepad_event::gamepad_event(void* _device_id, const gamepad::gamepad_state& _state) : event(EID_PAD), device_id(_device_id), state(_state)
		{
			flags = EF_MULTI;
		}
		/// return the device id, by default returns 0
		void* gamepad_event::get_device_id() const
		{
			return device_id;
		}
		/// write to stream
		void gamepad_event::stream_out(std::ostream& os) const
		{
			event::stream_out(os);
			unsigned i = (unsigned&)device_id;

			os << "id=" << i << " ";
			os << std::setprecision(2) << std::fixed
			   << state.trigger_position[0] << "|"
			   << std::showpos << state.left_stick_position[0] << "x"
			   << state.left_stick_position[1] << " <-> "
			   << std::noshowpos << state.trigger_position[1] << "|"
			   << std::showpos << state.right_stick_position[0] << "x"
			   << state.right_stick_position[1] << std::noshowpos
			   << std::setprecision(6) << std::resetiosflags(std::ios::fixed);
			if (state.button_flags > 0)
				os << " flags=" << convert_flags_to_string(gamepad::GamepadButtonStateFlags(state.button_flags));
		}
		// read from stream
		void gamepad_event::stream_in(std::istream&)
		{
			std::cerr << "event::stream_in not implemented yet" << std::endl;
		}

		gamepad_server::gamepad_server()
		{
			last_device_scan = 0;
			device_scan_interval = 2;
			gamepad::scan_devices();
			last_time_stamps.resize(gamepad::get_device_infos().size(), 0);
		}
		/// set time interval in seconds to check for device connection changes
		void gamepad_server::set_device_scan_interval(double duration)
		{
			device_scan_interval = duration;
		}
		/// check enabled gamepad devices for new events and dispatch them through the on_event signal
		void gamepad_server::check_and_emit_events(double time)
		{
			if (time > last_device_scan + device_scan_interval) {
				last_device_scan = time;
				size_t old_nr_devices = gamepad::get_device_infos().size();
				gamepad::scan_devices();
				if (gamepad::get_device_infos().size() != old_nr_devices) {
					last_time_stamps.resize(gamepad::get_device_infos().size());
					on_device_change(0);
				}
			}
			gamepad::GamepadKeys key;
			gamepad::KeyAction action;
			gamepad::gamepad_state state;
			// loop all devices
			for (unsigned device_index = 0; device_index < gamepad::get_device_infos().size(); ++device_index) {
				// if state query unsuccessful rescan devices and stop this for loop
				if (!get_state(device_index, state)) {
					gamepad::scan_devices();
					last_time_stamps.resize(gamepad::get_device_infos().size());
					on_device_change(device_index);
					break;
				}
				void* device_handle = 0;
				(int&)device_handle = device_index;
				// only show state in case of time stamp change
				if (state.time_stamp != last_time_stamps[device_index]) {
					last_time_stamps[device_index] = state.time_stamp;
					cgv::gui::gamepad_event gpe(device_handle, state);
					gpe.set_time(time);
					on_event(gpe);
				}

				while (query_key_event(device_index, key, action)) {
					char c = 0;
					if (key >= gamepad::GPK_A && key <= gamepad::GPK_Y)
						c = std::string("ABXY")[key - gamepad::GPK_A];
					gamepad_key_event gp_ke(device_handle, key, KeyAction(action - gamepad::KA_RELEASE + KA_RELEASE), c, time);
					on_event(gp_ke);
				}
			}
		}
		/// return a reference to gamepad server singleton
		gamepad_server& ref_gamepad_server()
		{
			static gamepad_server server;
			return server;
		}

		window_ptr& ref_dispatch_window_pointer()
		{
			static window_ptr w;
			return w;
		}
		bool dispatch_gamepad_event(cgv::gui::event& e)
		{
			return application::get_window(0)->dispatch_event(e);
		}

		/// connect the gamepad server to the given window or the first window of the application, if window is not provided
		void connect_gamepad_server(cgv::gui::window_ptr w)
		{
			if (w.empty())
				w = application::get_window(0);
			ref_dispatch_window_pointer() = w;
			connect(ref_gamepad_server().on_event, dispatch_gamepad_event);
			connect_copy(get_animation_trigger().shoot, cgv::signal::rebind(&ref_gamepad_server(), &gamepad_server::check_and_emit_events, cgv::signal::_1));
		}

	}
}