#include "gamepad_server.h"
#include <cgv/gui/application.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/trigger.h>
#include <iomanip>

namespace cgv {
	namespace gui {
		/// construct a key event from its textual description 
		gamepad_key_event::gamepad_key_event(void* _device_handle, const gamepad::gamepad_state& _state, unsigned short _key, KeyAction _action, unsigned char _char, double _time)
			: key_event(_key, _action, _char, 0, 0, _time), device_handle(_device_handle), state(_state)
		{
			flags = EF_PAD;
		}

		/// write to stream
		void gamepad_key_event::stream_out(std::ostream& os) const
		{
			event::stream_out(os);
			os << gamepad::get_key_string(key);
			switch (action) {
			case KA_RELEASE:
				os << " up ";
				break;
			case KA_PRESS:
				if (get_char())
					os << " = '" << get_char() << "'";
				break;
			case KA_REPEAT:
				os << " repeat ";
				break;
			}
			os << "*" << device_handle << "*";
		}

		/// construct a key event from its textual description 
		gamepad_throttle_event::gamepad_throttle_event(void* _device_handle, gamepad::gamepad_state& _state,
			float _x, float _dx, unsigned _player_index, unsigned _controller_index, unsigned _throttle_index, double _time)
			: throttle_event(_x,_dx,_player_index,_controller_index,_throttle_index, _time),
			device_handle(_device_handle), state(_state)
		{
			flags = EF_PAD;
		}
		/// write to stream
		void gamepad_throttle_event::stream_out(std::ostream& os) const
		{
			throttle_event::stream_out(os);
			os << "*" << device_handle << "*";
		}

		/// construct a key event from its textual description 
		gamepad_stick_event::gamepad_stick_event(void* _device_handle, gamepad::gamepad_state& _state,
			StickAction _action, float _x, float _y, float _dx, float _dy,
			unsigned _player_index, unsigned _controller_index, unsigned _stick_index, double _time)
			: stick_event(_action, _x, _y, _dx, _dy, _player_index, _controller_index, _stick_index, _time),
			device_handle(_device_handle), state(_state)
		{
			flags = EF_PAD;
		}
		/// return the device id, by default returns 0
		void* gamepad_stick_event::get_device_handle() const { return device_handle; }
		/// access to current gamepad state
		const gamepad::gamepad_state& gamepad_stick_event::get_state() const { return state; }

		/// write to stream
		void gamepad_stick_event::stream_out(std::ostream& os) const
		{
			stick_event::stream_out(os);
			os << "*" << device_handle << "*";
		}

		gamepad_server::gamepad_server()
		{
			last_device_scan = -1;
			device_scan_interval = 2;
			event_flags = GPE_ALL;
		}
		/// set time interval in seconds to check for device connection changes
		void gamepad_server::set_device_scan_interval(double duration)
		{
			device_scan_interval = duration;
		}
		/// set the event type flags of to be emitted events
		void gamepad_server::set_event_type_flags(GamepadEventTypeFlags flags)
		{
			event_flags = flags;
		}

		/// check enabled gamepad devices for new events and dispatch them through the on_event signal
		void gamepad_server::check_and_emit_events(double time)
		{
			if ((last_device_scan < 0) || (time > last_device_scan + device_scan_interval)) {
				last_device_scan = time;
				gamepad::scan_devices();
				std::vector<void*> new_device_handles;
				for (const auto& di : gamepad::get_device_infos())
					new_device_handles.push_back(di.device_handle);
				std::vector<gamepad::gamepad_state> new_states(new_device_handles.size());
				for (void* dh : device_handles) {
					auto iter = std::find(new_device_handles.begin(), new_device_handles.end(), dh);
					if (iter == new_device_handles.end()) {
						if ((event_flags&GPE_DEVICE) != 0)
							on_device_change(dh, false);
					}
					else {
						size_t i = iter - new_device_handles.begin();
						new_states[i] = last_states[i];
					}
				}
				for (void* ndh : new_device_handles) {
					auto iter = std::find(device_handles.begin(), device_handles.end(), ndh);
					if (iter == device_handles.end()) {
						if ((event_flags&GPE_DEVICE) != 0)
							on_device_change(ndh, true);
					}
				}
				last_states = new_states;
				device_handles = new_device_handles;
			}
			gamepad::GamepadKeys key;
			gamepad::KeyAction action;
			gamepad::gamepad_state state;
			// loop all devices
			for (unsigned i = 0; i < device_handles.size(); ++i) {
				// if state query unsuccessful continue loop
				if (!get_state(device_handles[i], state))
					continue;
				// process key events
				while (query_key_event(device_handles[i], key, action)) {
					if ((event_flags&GPE_KEY) != 0) {
						char c = 0;
						if (key >= gamepad::GPK_A && key <= gamepad::GPK_Y)
							c = std::string("ABXY")[key - gamepad::GPK_A];
						gamepad_key_event gp_ke(device_handles[i], state, key, KeyAction(action - gamepad::KA_RELEASE + KA_RELEASE), c, time);
						on_event(gp_ke);
					}
				}
				// only check for events in case of time stamp change
				if (state.time_stamp != last_states[i].time_stamp) {
					// check for throttle events
					if ((event_flags&GPE_THROTTLE) != 0) {
						for (int ti = 0; ti < 2; ++ti) {
							float x = state.trigger_position[ti];
							float dx = x - last_states[i].trigger_position[ti];
							if (dx != 0) {
								gamepad_throttle_event gp_te(device_handles[i], state, x, dx,
									i, 0, ti, time);
								on_event(gp_te);
							}
						}
					}
					// check for stick press / release events
					if ((event_flags&(GPE_STICK+GPE_STICK_KEY)) != 0) {
						for (int si = 0; si < 2; ++si) {
							unsigned stick_flag = (si == 0 ? gamepad::GBF_LEFT_STICK : gamepad::GBF_RIGHT_STICK);
							if ((state.button_flags&stick_flag) != (last_states[i].button_flags&stick_flag)) {
								float x = (si == 0 ? state.left_stick_position[0] : state.right_stick_position[0]);
								float y = (si == 0 ? state.left_stick_position[1] : state.right_stick_position[1]);
								if ((event_flags&GPE_STICK) != 0) {
									StickAction action = ((state.button_flags&stick_flag) != 0 ? SA_PRESS : SA_RELEASE);
									gamepad_stick_event gp_se(device_handles[i], state, action, x, y, 0, 0, i, 0, si, time);
									on_event(gp_se);
								}
								if ((event_flags&GPE_STICK_KEY) != 0) {
									int qx = x > 0.5f ? 1 : (x < -0.5f ? -1 : 0);
									int qy = y > 0.5f ? 1 : (y < -0.5f ? -1 : 0);
									short key_index = si*9 + 3 * (qy+1) + qx+1;
									static gamepad::GamepadKeys key_lookup[18] = {
										gamepad::GPK_LEFT_STICK_DOWNLEFT,
										gamepad::GPK_LEFT_STICK_DOWN,
										gamepad::GPK_LEFT_STICK_DOWNRIGHT,
										gamepad::GPK_LEFT_STICK_LEFT,
										gamepad::GPK_LEFT_STICK_PRESS,
										gamepad::GPK_LEFT_STICK_RIGHT,
										gamepad::GPK_LEFT_STICK_UPLEFT,
										gamepad::GPK_LEFT_STICK_UP,
										gamepad::GPK_LEFT_STICK_UPRIGHT,
										gamepad::GPK_RIGHT_STICK_DOWNLEFT,
										gamepad::GPK_RIGHT_STICK_DOWN,
										gamepad::GPK_RIGHT_STICK_DOWNRIGHT,
										gamepad::GPK_RIGHT_STICK_LEFT,
										gamepad::GPK_RIGHT_STICK_PRESS,
										gamepad::GPK_RIGHT_STICK_RIGHT,
										gamepad::GPK_RIGHT_STICK_UPLEFT,
										gamepad::GPK_RIGHT_STICK_UP,
										gamepad::GPK_RIGHT_STICK_UPRIGHT
									};
									KeyAction action = ((state.button_flags&stick_flag) != 0 ? KA_PRESS : KA_RELEASE);
									gamepad_key_event gp_ke(device_handles[i], state, 
										key_lookup[key_index], action, 0, time);
									on_event(gp_ke);
								}
							}
						}
					}
					// check for stick move and drag events
					if ((event_flags&GPE_STICK) != 0) {
						float x = state.left_stick_position[0];
						float y = state.left_stick_position[1];
						float dx = x - last_states[i].left_stick_position[0];
						float dy = y - last_states[i].left_stick_position[1];
						if (dx != 0 || dy != 0) {
							StickAction action = SA_MOVE;
							if ((state.button_flags & gamepad::GBF_LEFT_STICK) != 0)
								action = SA_DRAG;

							gamepad_stick_event gp_se(device_handles[i], state, 
								action, x, y, dx, dy, i, 0, 0, time);
							on_event(gp_se);
						}
						x = state.right_stick_position[0];
						y = state.right_stick_position[1];
						dx = x - last_states[i].right_stick_position[0];
						dy = y - last_states[i].right_stick_position[1];
						if (dx != 0 || dy != 0) {
							StickAction action = SA_MOVE;
							if ((state.button_flags & gamepad::GBF_RIGHT_STICK) != 0)
								action = SA_DRAG;

							gamepad_stick_event gp_se(device_handles[i], state,
								action, x, y, dx, dy, i, 0, 1, time);
							on_event(gp_se);
						}
					}
					last_states[i] = state;
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