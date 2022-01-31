#include "vr_events.h"
#include <cassert>

namespace cgv {
	namespace gui {
		/// construct extension from device handle and vr state
		vr_event_extension::vr_event_extension(void* _device_handle, const vr::vr_kit_state& _state) 
			: device_handle(_device_handle), state(_state)
		{

		}
		/// construct a key event from its textual description 
		vr_key_event::vr_key_event(void* _device_handle, unsigned _player_index, unsigned _controller_index, const vr::vr_kit_state& _state,
			unsigned short _key, KeyAction _action, unsigned char _char, 
			unsigned char _modifiers, double _time)
			: vr_event_extension(_device_handle, _state), controller_index(_controller_index), player_index(_player_index), 
			  key_event(_key, _action, _char, _modifiers, 0, _time)
		{
			flags = EF_VR;
		}
		/// write to stream
		void vr_key_event::stream_out(std::ostream& os) const
		{
			event::stream_out(os);
			os << vr::get_key_string(key);
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
			if (get_modifiers() != 0) {
				os << " {" << vr::get_state_flag_string(vr::VRButtonStateFlags(get_modifiers())) << "}";
			}
			os << "<" << (unsigned)player_index << ":" << (unsigned)controller_index << ">";
			os << "*" << device_handle << "*";
		}
		/// read from stream
		void vr_key_event::stream_in(std::istream& is)
		{
			key_event::stream_in(is);
		}

		/// construct a throttle event from value and value change
		vr_throttle_event::vr_throttle_event(void* _device_handle, unsigned _controller_index, const vr::vr_kit_state& _state,
			float _x, float _dx, unsigned _player_index, unsigned _throttle_index, double _time)
			: throttle_event(_x, _dx, _player_index, _controller_index, _throttle_index, _time),
			vr_event_extension(_device_handle, _state)
		{
			flags = EF_VR;
		}
		/// write to stream
		void vr_throttle_event::stream_out(std::ostream& os) const
		{
			throttle_event::stream_out(os);
			os << "*" << device_handle << "*";
		}
		/// construct a key event from its textual description 
		vr_stick_event::vr_stick_event(void* _device_handle, unsigned _controller_index, const vr::vr_kit_state& _state,
			StickAction _action, float _x, float _y, float _dx, float _dy,
			unsigned _player_index, unsigned _stick_index, double _time)
			: stick_event(_action, _x, _y, _dx, _dy, _player_index,_controller_index,_stick_index,_time),
			vr_event_extension(_device_handle, _state)
		{
			flags = EF_VR;
		}
		/// write to stream
		void vr_stick_event::stream_out(std::ostream& os) const
		{
			stick_event::stream_out(os);
			os << "*" << device_handle << "*";
		}
		/// construct a key event from its textual description 
		vr_pose_event::vr_pose_event(void* _device_handle, short _trackable_index, const vr::vr_kit_state& _state,
			const float *_pose, const float *_last_pose, unsigned short _player_index, double _time)
			: pose_event(_pose, _last_pose, _player_index, _trackable_index, _time),
			vr_event_extension(_device_handle, _state)
		{
			flags = EF_VR;
		}
		/// write to stream
		void vr_pose_event::stream_out(std::ostream& os) const
		{
			pose_event::stream_out(os);
			os << "*" << device_handle << "*";
		}
	}
}