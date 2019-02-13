#include <vr/vr_driver.h>
#include "vr_server.h"
#include <cgv/gui/application.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/trigger.h>
#include <cassert>

namespace cgv {
	namespace gui {
		/// construct a gamepad event
		vr_event::vr_event(void* _device_handle, const vr::vr_kit_state& _state, double _time) :
			device_handle(_device_handle), state(_state), cgv::gui::event(cgv::gui::EID_VR, 0, 0, time)
		{
		}
		/// return the device id, by default returns 0
		void* vr_event::get_device_handle() const {
			return device_handle;
		}
		/// return the state
		const vr::vr_kit_state& vr_event::get_state() const {
			return state;
		}
		/// 
		void vr_event::stream_out_trackable(std::ostream& os, const vr::vr_trackable_state& trackable) const
		{
			os << "{" << vr::get_status_string(trackable.status) << "}";
			for (unsigned i = 0; i < 12; ++i)
				os << " " << trackable.pose[i];
		}
		/// write to stream
		void vr_event::stream_out(std::ostream& os) const {
			cgv::gui::event::stream_out(os);
			os << " handle[" << device_handle << "]\n" << "   hmd: ";
			stream_out_trackable(os, state.hmd);
			os << "\n   left controller [" << std::hex << state.controller[0].time_stamp
				<< "] <" << state.controller[0].button_flags << ">: ";
			stream_out_trackable(os, state.controller[0]);
			os << "\n  right controller [" << std::hex << state.controller[1].time_stamp
				<< "] <" << state.controller[1].button_flags << ">: ";
			stream_out_trackable(os, state.controller[1]);
			os << std::endl;
		}
		/// read from stream
		void vr_event::stream_in(std::istream& is) {
			std::cerr << "stream in of vr event not implemented" << std::endl;
			cgv::gui::event::stream_in(is);
		}
		/// construct a key event from its textual description 
		vr_key_event::vr_key_event(void* _device_handle, int _controller_index, unsigned short _key, KeyAction _action, unsigned char _char, double _time)
			: device_handle(_device_handle), controller_index(_controller_index), key_event(_key, _action, _char, 0, 0, time)
		{
		}
		/// return controller index (0 .. left, 1.. right) of vr kit
		int vr_key_event::get_controller_index() const
		{
			return controller_index;
		}
		/// return the device id, by default returns 0
		void* vr_key_event::get_device_handle() const
		{
			return device_handle;
		}
		/// write to stream
		void vr_key_event::stream_out(std::ostream& os) const
		{
			event::stream_out(os);
			os << vr::get_key_string(key);
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
			os << "[" << device_handle << "]";
		}
		/// read from stream
		void vr_key_event::stream_in(std::istream& is)
		{
			key_event::stream_in(is);
		}

			double last_device_scan;
			double device_scan_interval;
			std::vector<vr::vr_kit_state> last_states;
			std::vector<void*> vr_kit_handles;
			/// construct server with default configuration
			vr_server::vr_server()
			{
				last_device_scan = 0;
				device_scan_interval = 5;
				vr_kit_handles = vr::scan_vr_kits();
				last_time_stamps.resize(vr_kit_handles.size(), 0);
			}
			/// set time interval in seconds to check for device connection changes
			void vr_server::set_device_scan_interval(double duration)
			{
				device_scan_interval = duration;
			}
			/// check enabled gamepad devices for new events and dispatch them through the on_event signal
			void vr_server::check_and_emit_events(double time)
			{
				std::vector<bool> is_first_state(vr_kit_handles.size(), false);
				if (time > last_device_scan + device_scan_interval) {
					last_device_scan = time;
					std::vector<void*> new_handles = vr::scan_vr_kits();
					std::vector<vr::vr_kit_state> new_last_states;
					new_last_states.resize(new_handles.size());
					is_first_state.resize(new_handles.size(), false);
					for (void* h1 : vr_kit_handles)
						if (std::find(new_handles.begin(), new_handles.end(), h1) == new_handles.end())
							on_device_change(h1, false);
					unsigned i = 0;
					for (void* h2 : new_handles) {
						auto iter = std::find(vr_kit_handles.begin(), vr_kit_handles.end(), h2);
						if (iter == vr_kit_handles.end()) {
							on_device_change(h2, true);
							is_first_state.at(i) = true;
						}
						else {
							new_last_states[i] = last_states.at(iter - vr_kit_handles.begin());
						}
						++i;
					}
					vr_kit_handles = new_handles;
					last_states = new_last_states;
				}
				// loop all devices
				unsigned i;
				for (i = 0; i < vr_kit_handles.size(); ++i) {
					vr::vr_kit* kit = vr::get_vr_kit(vr_kit_handles[i]);
					if (!kit)
						continue;
					vr::vr_kit_state state;
					kit->query_state(state, 1);
					if (!(state == last_states[i])) {
						cgv::gui::vr_event vre(vr_kit_handles[i], state, time);
						on_event(vre);
					}
					last_states[i] = state;
				}
			}
		/// return a reference to gamepad server singleton
		vr_server& ref_vr_server()
		{
			static vr_server server;
			return server;
		}

		window_ptr& ref_dispatch_window_pointer()
		{
			static window_ptr w;
			return w;
		}
		bool dispatch_vr_event(cgv::gui::event& e)
		{
			return application::get_window(0)->dispatch_event(e);
		}

		/// connect the gamepad server to the given window or the first window of the application, if window is not provided
		void connect_vr_server(cgv::gui::window_ptr w)
		{
			if (w.empty())
				w = application::get_window(0);
			ref_dispatch_window_pointer() = w;
			connect(ref_vr_server().on_event, dispatch_vr_event);
			connect_copy(get_animation_trigger().shoot, cgv::signal::rebind(&ref_vr_server(), &vr_server::check_and_emit_events, cgv::signal::_1));
		}

	}
}