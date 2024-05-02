#include <cgv/base/base.h>
#include <vr/vr_driver.h>
#include "vr_server.h"
#include "vr_events.h"
#include <cgv/gui/application.h>
#include <cgv/signal/rebind.h>
#include <cgv/gui/trigger.h>
#include <cgv/gui/choice_event.h>
#include <cassert>

namespace cgv {
	namespace gui {
		// some helper function to compare arrays
		bool array_unequal(const float* a1, const float* a2, unsigned n) {
			for (unsigned i = 0; i < n; ++i)
				if (a1[i] != a2[i])
					return true;
			return false;
		}
		/// construct server with default configuration
		vr_server::vr_server()
		{	
			last_device_scan = -1;
			device_scan_interval = 1;
			event_type_flags = VRE_ALL;
			last_time_stamps.resize(vr_kit_handles.size(), 0);
		}
		/// set time interval in seconds to check for device connection changes
		void vr_server::set_device_scan_interval(double duration)
		{
			device_scan_interval = duration;
		}
		struct button_mapping
		{
			vr::VRButtonStateFlags flag;
			vr::VRKeys key;
		};
		/// grab the event focus to the given event handler and return whether this was possible
		bool vr_server::grab_focus(VRFocus _focus_type, event_handler* handler)
		{
			choice_event ce(CET_LOOSE_FOCUS, 0, 0, trigger::get_current_time());
			ce.set_flags(EF_VR);
			if (focus) {
				if (handler == focus)
					return true;
				if ((focus_type & VRF_PERMANENT) != 0)
					return false;
				focus->handle(ce);
				focus = 0;
			}
			ce.set_type(CET_GRAB_FOCUS);
			focus = handler;
			focus_type = _focus_type;
			focus->handle(ce);
			return true;
		}
		/// release focus of handler and return whether handler had the focus
		bool vr_server::release_focus(event_handler* handler)
		{
			if (!focus || focus != handler)
				return false;
			focus_type = VRF_RELEASED;
			focus = 0;
			return true;
		}
		/// 
		VREventTypeFlags vr_server::get_event_type_flags() const
		{
			return event_type_flags;
		}
		/// 
		void vr_server::set_event_type_flags(VREventTypeFlags flags)
		{
			event_type_flags = flags;
		}
		///
		float vr_server::correct_deadzone_and_precision(float value, const vr::controller_input_config& IC)
		{
			if (IC.precision > 0)
				value = IC.precision * floor(value / IC.precision + 0.5f);
			if (IC.dead_zone > 0)
				if (std::abs(value) < IC.dead_zone)
					value = 0;
				else
					value = (value - IC.dead_zone) / (1.0f - IC.dead_zone);
			return value;
		}
		///
		vr_server::vec2 vr_server::correct_deadzone_and_precision(const vec2& position, const vr::controller_input_config& IC)
		{
			vec2 result = position;
			if (IC.precision > 0)
				result = IC.precision*round(result/IC.precision+0.5f);
			if (IC.dead_zone > 0)
				if (result.length() < IC.dead_zone)
					result.zeros();
			return result;
		}
		///
		void vr_server::emit_events_and_update_state(void* kit_handle, const vr::vr_kit_state& new_state, int kit_index, VREventTypeFlags flags, double time)
		{
			static button_mapping button_keys[19] = {
				{ vr::VRF_SYSTEM,       vr::VR_SYSTEM },
				{ vr::VRF_MENU,         vr::VR_MENU },
				{ vr::VRF_GRIP,         vr::VR_GRIP },
				{ vr::VRF_DPAD_LEFT,    vr::VR_DPAD_LEFT },
				{ vr::VRF_DPAD_RIGHT,   vr::VR_DPAD_RIGHT },
				{ vr::VRF_DPAD_DOWN,    vr::VR_DPAD_DOWN },
				{ vr::VRF_DPAD_UP,      vr::VR_DPAD_UP },
				{ vr::VRF_A,            vr::VR_A },
				{ vr::VRF_INPUT0_TOUCH, vr::VR_INPUT0_TOUCH },
				{ vr::VRF_INPUT0,       vr::VR_INPUT0 },
				{ vr::VRF_INPUT1_TOUCH, vr::VR_INPUT1_TOUCH },
				{ vr::VRF_INPUT1,       vr::VR_INPUT1 },
				{ vr::VRF_INPUT2_TOUCH, vr::VR_INPUT2_TOUCH },
				{ vr::VRF_INPUT2,       vr::VR_INPUT2 },
				{ vr::VRF_INPUT3_TOUCH, vr::VR_INPUT3_TOUCH },
				{ vr::VRF_INPUT3,       vr::VR_INPUT3 },
				{ vr::VRF_INPUT4_TOUCH, vr::VR_INPUT4_TOUCH },
				{ vr::VRF_INPUT4,       vr::VR_INPUT4 },
				{ vr::VRF_PROXIMITY,    vr::VR_PROXIMITY }
			};
			vr::vr_kit* kit_ptr = vr::get_vr_kit(kit_handle);
			vr::vr_kit_state& last_state = last_states[kit_index];
			// check for status changes and emit signal for found status changes					
			if ((flags & VRE_STATUS) != 0) {
				if (new_state.hmd.status != last_state.hmd.status)
					on_status_change(kit_handle, -1, last_state.hmd.status, new_state.hmd.status);
				for (int ci = 0; ci < vr::max_nr_controllers; ++ci) {
					if (new_state.controller[ci].status != last_state.controller[ci].status)
						on_status_change(kit_handle, ci, last_state.controller[ci].status, new_state.controller[ci].status);
				}
			}
			// check for key changes and emit key_event 
			if ((flags & (VRE_KEY | VRE_ONE_AXIS_GENERATES_KEY | VRE_TWO_AXES_GENERATES_DPAD)) != 0) {
				for (int ci = 0; ci < vr::max_nr_controllers; ++ci) {
					const auto& CS_new = new_state.controller[ci];
					const auto& CS_lst = last_state.controller[ci];
					// ignore cases when controller changed attachment
					if (CS_new.status == vr::VRS_DETACHED || CS_lst.status == vr::VRS_DETACHED)
						continue;
					// ensure that a key changed
					if (CS_new.button_flags == CS_lst.button_flags)
						continue;
					// first check direct key events
					if ((flags & VRE_KEY) != 0) {
						for (unsigned j = 0; j < 19; ++j) {
							if ((button_keys[j].flag == vr::VRF_INPUT0) && ((flags & VRE_TWO_AXES_GENERATES_DPAD) != 0))
								continue;
							if ((CS_new.button_flags & button_keys[j].flag) != (CS_lst.button_flags & button_keys[j].flag)) {
								// construct and emit event
								vr_key_event vrke(kit_handle, kit_index, ci, new_state,
									button_keys[j].key,
									(CS_new.button_flags & button_keys[j].flag) != 0 ? KA_PRESS : KA_RELEASE,
									0, 0, time);
								dispatch(vrke);
							}
						}
					}
					// next check for dpad events
					if (((flags & VRE_TWO_AXES_GENERATES_DPAD) != 0) && ((CS_new.button_flags & vr::VRF_INPUT0) != (CS_lst.button_flags & vr::VRF_INPUT0))) {
						// dpad keys are ordered from left to right and bottom to top with VR_INPUT0 in center
						// compute key offset relative to VR_INPUT0
						short key_offset = 0;
						float th = kit_ptr ? kit_ptr->get_controller_input_config(ci, 0).threshold : 0.5f;
						int x = CS_new.axes[0] > th ? 1 : (CS_new.axes[0] < -th ? -1 : 0);
						int y = CS_new.axes[1] > th ? 1 : (CS_new.axes[1] < -th ? -1 : 0);
						//std::cout << x << "|" << CS_new.axes[0] << ":" << th << "; " << y << "|" << CS_new.axes[1] << ":" << th << std::endl;
						key_offset = 3 * y + x;
						vr::VRKeys key = vr::VR_INPUT0;
						if (key_offset != 0) {
							if (key_offset > 0)
								--key_offset;
							key = vr::VRKeys(vr::VR_DPAD_RIGHT + key_offset);
						}
						// construct and emit event
						vr_key_event vrke(kit_handle, kit_index, ci, new_state, key,
							(CS_new.button_flags & vr::VRF_INPUT0) != 0 ? KA_PRESS : KA_RELEASE, 0,	0, time);
						dispatch(vrke);
					}
				}
			}
			// check for axes input events
			if (kit_ptr && ((flags & (VRE_ONE_AXIS|VRE_TWO_AXES)) != 0)) {
				for (int ci = 0; ci < vr::max_nr_controllers; ++ci) {
					const auto& CS_new = new_state.controller[ci];
					const auto& CS_lst = last_state.controller[ci];
					// iterate all controller inputs
					vr::VRButtonStateFlags input_flag = vr::VRF_INPUT0;
					vr::VRButtonStateFlags touch_flag = vr::VRF_INPUT0_TOUCH;
					int ai = 0;
					for (int ii = 0; ii < vr::max_nr_controller_inputs; ++ii,
						input_flag = vr::VRButtonStateFlags(4 * input_flag),
						touch_flag = vr::VRButtonStateFlags(4 * touch_flag)) {
						// determine input type
						vr::VRInputType it = kit_ptr->get_device_info().controller[ci].input_type[ii];
						if (it == vr::VRI_NONE)
							continue;
						if (it == vr::VRI_TRIGGER) {
							if ((flags & VRE_ONE_AXIS) != 0) {
								const auto& CIC = kit_ptr->get_controller_input_config(ci, ii);
								float x = correct_deadzone_and_precision(CS_new.axes[ai], CIC);
								float dx = x - correct_deadzone_and_precision(CS_lst.axes[ai], CIC);
								if (dx != 0) {
									/// construct a throttle event from value and value change
									vr_throttle_event vrte(kit_handle, ci, new_state, x, dx, kit_index, ii, time);
									dispatch(vrte);
								}
							}
							++ai;
							continue;
						}
						// check for pad/stick touch / press / release / untouch events
						else {
							if ((flags & VRE_TWO_AXES) != 0) {
								// check if press/touch state changed
								int new_p_or_t = ((CS_new.button_flags & touch_flag) != 0) ? 1 : 0;
								if ((CS_new.button_flags & input_flag) != 0)
									new_p_or_t = 2;
								int old_p_or_t = ((CS_lst.button_flags & touch_flag) != 0) ? 1 : 0;
								if ((CS_lst.button_flags & input_flag) != 0)
									old_p_or_t = 2;
								if (new_p_or_t != old_p_or_t) {
									static StickAction actions[9] = {
										SA_MOVE, SA_TOUCH, SA_PRESS,
										SA_RELEASE, SA_MOVE, SA_PRESS,
										SA_RELEASE, SA_UNPRESS, SA_MOVE
									};
									StickAction action = actions[new_p_or_t + 3 * old_p_or_t];
									float x = new_state.controller[ci].axes[ai];
									float y = new_state.controller[ci].axes[ai + 1];
									vr_stick_event vrse(kit_handle, ci, new_state,
										action, x, y, 0, 0, kit_index, 0, time);
									dispatch(vrse);
								}
								// otherwise check for move or drag event
								else {
									const auto& CIC = kit_ptr->get_controller_input_config(ci, ii);
									vec2 p(CS_new.axes[ai], CS_new.axes[ai + 1]);
									p = correct_deadzone_and_precision(p, CIC);
									vec2 last_p(CS_lst.axes[ai], CS_lst.axes[ai + 1]);
									last_p = correct_deadzone_and_precision(last_p, CIC);
									vec2 diff = p - last_p;
									if (diff(0) != 0 || diff(1) != 0) {
										StickAction action = SA_MOVE;
										if ((CS_new.button_flags & input_flag) != 0)
											action = SA_DRAG;
										vr_stick_event vrse(kit_handle, ci, new_state,
											action, p(0), p(1), diff(0), diff(1), kit_index, ii, time);
										dispatch(vrse);
									}
								}
							}
							ai += 2;
							continue;
						}
					}
				}
			}
			// finally check for pose events
			if ((flags & VRE_POSE) != 0) {
				if (array_unequal(new_state.hmd.pose, last_state.hmd.pose, 12)) {
					vr_pose_event vrpe(kit_handle, -1, new_state, new_state.hmd.pose, last_state.hmd.pose, kit_index, time);
					dispatch(vrpe);
				}
				for (int c = 0; c < vr::max_nr_controllers; ++c) 
					if (new_state.controller[c].status == vr::VRS_TRACKED) {
						if (array_unequal(new_state.controller[c].pose, last_state.controller[c].pose, 12)) {
							vr_pose_event vrpe(kit_handle, c, new_state, new_state.controller[c].pose, last_state.controller[c].pose, kit_index, time);
							dispatch(vrpe);
						}
					}
			}

			//write log
			if (log_data[kit_index] && !(new_state == last_state)) {
				log_data[kit_index]->log_vr_state(new_state,time);
			}
			last_state = new_state;
		}

		void vr_server::check_device_changes(double time)
		{
			std::vector<bool> is_first_state(vr_kit_handles.size(), false);
			if (last_device_scan < 0 ||
				((device_scan_interval > 0) && (time > last_device_scan + device_scan_interval))) {
				last_device_scan = time;
				std::vector<void*> new_handles = vr::scan_vr_kits();
				std::vector<vr::vr_kit_state> new_last_states(new_handles.size());
				is_first_state.resize(new_handles.size(), false);
				// detect device disconnect events
				if ((get_event_type_flags() & VRE_DEVICE) != 0)
					for (void* h1 : vr_kit_handles)
						if (std::find(new_handles.begin(), new_handles.end(), h1) == new_handles.end())
							on_device_change(h1, false);
				// keep vector of parameter pairs for on_device change events
				std::vector<std::pair<void*, bool> > on_device_change_params;
				// go through new devices
				unsigned i = 0;
				for (void* h2 : new_handles) {
					auto iter = std::find(vr_kit_handles.begin(), vr_kit_handles.end(), h2);
					// detect device connect events
					if (iter == vr_kit_handles.end()) {
						if ((get_event_type_flags() & VRE_DEVICE) != 0)
							on_device_change_params.push_back(std::pair<void*, bool>(h2, true));
						is_first_state.at(i) = true;
					}
					else {
						new_last_states[i] = last_states.at(iter - vr_kit_handles.begin());
					}
					++i;
				}
				vr_kit_handles = new_handles;
				last_states = new_last_states;
				for (auto pp : on_device_change_params)
					on_device_change(pp.first, pp.second);
			}
		}
		/// check enabled gamepad devices for new events and dispatch them through the on_event signal
		void vr_server::check_and_emit_events(double time)
		{
			check_device_changes(time);
			// loop all devices
			unsigned i;
			for (i = 0; i < vr_kit_handles.size(); ++i) {
				vr::vr_kit* kit = vr::get_vr_kit(vr_kit_handles[i]);
				if (!kit)
					continue;
				// query current state
				vr::vr_kit_state state;
				kit->query_state(state, 1);
				emit_events_and_update_state(vr_kit_handles[i], state, i, event_type_flags, time);
			}
		}

		/// in case the current vr state of a kit had been queried outside, use this function to communicate the new state to the server 
		bool vr_server::check_new_state(void* kit_handle, const vr::vr_kit_state& new_state, double time)
		{
			return check_new_state(kit_handle, new_state, time, event_type_flags);
		}
		/// same as previous function but with overwrite of flags
		bool vr_server::check_new_state(void* kit_handle, const vr::vr_kit_state& new_state, double time, VREventTypeFlags flags)
		{
			auto iter = std::find(vr_kit_handles.begin(), vr_kit_handles.end(), kit_handle);
			if (iter == vr_kit_handles.end())
				return false;
			size_t i = iter - vr_kit_handles.begin();
			emit_events_and_update_state(kit_handle, new_state, (int)i, flags, time);
			return true;
		}
		bool vr_server::dispatch(cgv::gui::event& e)
		{
			if (focus && focus_type != VRF_RELEASED) {
				if (focus->handle(e))
					return true;
				if ((focus_type & VRF_EXCLUSIVE) != 0)
					return false;
			}
			return on_event(e);
		}
		void vr_server::enable_log(std::string fn, bool in_memory_log, int filter, int kit_index)
		{
			auto it = log_data.find(kit_index);
			if (log_data[kit_index]) {
				log_data[kit_index]->disable_log();
				log_data[kit_index] = nullptr;
			}
			log_data[kit_index] = new vr::vr_log();
			vr::vr_log& log = *log_data[kit_index];
			
			if (fn.size() > 0) {
				auto p = std::make_shared<std::ofstream>(fn);
				log.enable_ostream_log(p);
			}
			if (in_memory_log)
				log.enable_in_memory_log();

			log.set_filter(filter);
			log.lock_settings();
		}
		void vr_server::disable_log(int kit_index)
		{
			auto it = log_data.find(kit_index);
			if (it != log_data.end() && it->second)
				it->second->disable_log();
		}

		vr::vr_log& vr_server::ref_log(const int kit_index)
		{
			return *log_data[kit_index];
		}
		cgv::data::ref_ptr<vr::vr_log> vr_server::get_log(const int kit_index)
		{
			return log_data[kit_index];
		}
		/// return a reference to gamepad server singleton
		vr_server& ref_vr_server()
		{
			static vr_server server;
			return server;
		}

		window_ptr& ref_dispatch_window_pointer_vr()
		{
			static window_ptr w;
			return w;
		}
		bool dispatch_vr_event(cgv::gui::event& e)
		{
			return application::get_window(0)->dispatch_event(e);
		}

		/// connect the gamepad server to the given window or the first window of the application, if window is not provided
		void connect_vr_server(bool connect_device_change_only_to_animation_trigger, cgv::gui::window_ptr w)
		{
			if (w.empty())
				w = application::get_window(0);
			if (ref_dispatch_window_pointer_vr() != w) {
				ref_dispatch_window_pointer_vr() = w;
				connect(ref_vr_server().on_event, dispatch_vr_event);
				if (connect_device_change_only_to_animation_trigger)
					connect_copy(get_animation_trigger().shoot, cgv::signal::rebind(&ref_vr_server(), &vr_server::check_device_changes, cgv::signal::_1));
				else
					connect_copy(get_animation_trigger().shoot, cgv::signal::rebind(&ref_vr_server(), &vr_server::check_and_emit_events, cgv::signal::_1));
			}
		}

	}
}