#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/key_event.h>
#include <cgv/gui/window.h>
#include <cgv/signal/signal.h>
#include <cgv/signal/bool_signal.h>
#include <libs/gamepad/gamepad.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {
		/// gamepad key events use the key codes defined in gamepad::GamepadKeys
		class CGV_API gamepad_key_event : public cgv::gui::key_event
		{
		protected:
			/// store id of gamepad
			void* device_id;
		public:
			/// construct a key event from its textual description 
			gamepad_key_event(void* _device_id, unsigned short _key = 0, KeyAction _action = KA_PRESS, unsigned char _char = 0, double _time = 0);
			/// return the device id, by default returns 0
			void* get_device_id() const;
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
		};
		
		/// the gamepad event is triggered whenever a gamepad state changes
		class CGV_API gamepad_event : public cgv::gui::event
		{
		protected:
			/// store id of gamepad
			void* device_id;
			/// 
			gamepad::gamepad_state state;
		public:
			/// construct a gamepad event
			gamepad_event(void* _device_id, const gamepad::gamepad_state& state);
			/// return reference to game stae
			const gamepad::gamepad_state& get_state() const;
			/// return the device id, by default returns 0
			void* get_device_id() const;
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
		};

		class CGV_API gamepad_server
		{
		protected:
			double last_device_scan;
			double device_scan_interval;
			std::vector<unsigned> last_time_stamps;
		public:
			/// construct server with default configuration
			gamepad_server();
			/// set time interval in seconds to check for device connection changes
			void set_device_scan_interval(double duration);
			/// check enabled gamepad devices for new events and dispatch them through the on_event signal
			void check_and_emit_events(double time);
			/// signal emitted to dispatch events
			cgv::signal::bool_signal<cgv::gui::event&> on_event;
			/// signal emitted to notify about device changes
			cgv::signal::signal<int> on_device_change;
		};

		/// return a reference to gamepad server singleton
		extern CGV_API gamepad_server& ref_gamepad_server();

		/// connect the gamepad server to the given window or the first window of the application, if window is not provided
		extern CGV_API void connect_gamepad_server(cgv::gui::window_ptr w = cgv::gui::window_ptr());
	}
}

#include <cgv/config/lib_end.h>
