#pragma once

#include <cgv/gui/event.h>
#include <cgv/gui/key_event.h>
#include <vr/vr_event.h>
#include <cgv/gui/window.h>
#include <cgv/signal/signal.h>
#include <cgv/signal/bool_signal.h>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

		/// the vr event is triggered whenever a vr_kit state changes
		class CGV_API vr_event : public cgv::gui::event
		{
		public:
			/// store id of gamepad
			void* device_handle;
			/// public access to game state allows skipping of library dependency
			vr::vr_kit_state state;
			/// 
			void stream_out_trackable(std::ostream& os, const vr::vr_trackable_state& trackable) const;
			/// 
			void stream_out_controller(std::ostream& os, const vr::vr_controller_state& controller) const;
		public:
			/// construct a gamepad event
			vr_event(void* _device_handle, const vr::vr_kit_state& _state, double _time = 0);
			/// return the device id, by default returns 0
			void* get_device_handle() const;
			/// return the state
			const vr::vr_kit_state& get_state() const;
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
		};

		/// vr key events use the key codes defined in vr::GamepadKeys
		class CGV_API vr_key_event : public cgv::gui::key_event
		{
		protected:
			/// store controller index (0 .. left, 1.. right) of vr kit
			int controller_index;
			/// store device handle
			void* device_handle;
		public:
			/// construct a key event from its textual description 
			vr_key_event(void* _device_handle, int _controller_index, 
				unsigned short _key = 0, KeyAction _action = KA_PRESS, unsigned char _char = 0, 
				unsigned char _modifiers = 0, double _time = 0);
			/// return controller index (0 .. left, 1.. right) of vr kit
			int get_controller_index() const;
			/// return the device id, by default returns 0
			void* get_device_handle() const;
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
		};

		class CGV_API vr_server
		{
		protected:
			double last_device_scan;
			double device_scan_interval;
			std::vector<void*> vr_kit_handles;
			std::vector<vr::vr_kit_state> last_states;
			std::vector<unsigned> last_time_stamps;
		public:
			/// construct server with default configuration
			vr_server();
			/// 
			void on_set(void* member_ptr);
			/// set time interval in seconds to check for device connection changes
			void set_device_scan_interval(double duration);
			/// check enabled gamepad devices for new events and dispatch them through the on_event signal
			void check_and_emit_events(double time);
			/// signal emitted to dispatch events
			cgv::signal::bool_signal<cgv::gui::event&> on_event;
			/// signal emitted to notify about device changes, first argument is handle and second a flag telling whether device got connected or if false disconnected
			cgv::signal::signal<void*, bool> on_device_change;
			/// signal emitted to notify about status changes of trackables, first argument is handle, second -1 for hmd + 0|1 for left|right controller, third is old status and fourth new status
			cgv::signal::signal<void*, int, vr::VRStatus, vr::VRStatus> on_status_change;
		};

		/// return a reference to vr server singleton
		extern CGV_API vr_server& ref_vr_server();

		/// connect the vr server to the given window or the first window of the application, if window is not provided
		extern CGV_API void connect_vr_server(cgv::gui::window_ptr w = cgv::gui::window_ptr());
	}
}

#include <cgv/config/lib_end.h>
