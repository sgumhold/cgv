#pragma once

#include <cgv/gui/key_event.h>
#include <cgv/gui/throttle_event.h>
#include <cgv/gui/stick_event.h>
#include <cgv/gui/pose_event.h>
#include <vr/vr_state.h>
#include <cgv/gui/window.h>
#include <cgv/signal/signal.h>
#include <cgv/signal/bool_signal.h>

#include "lib_begin.h"

///@ingroup VR
///@{

///
namespace cgv {
	///
	namespace gui {

		/// vr key events use the key codes defined in vr::VRKeys
		class CGV_API vr_key_event : public cgv::gui::key_event
		{
		protected:
			/// store device handle
			void* device_handle;
			/// store controller index (0 .. left, 1.. right, 2 .. fst generic, 3 .. snd generic) of vr kit
			unsigned short controller_index;
			/// store player index
			unsigned short player_index;
			/// access to current vr state 
			const vr::vr_kit_state& state;
		public:
			/// construct a key event from its textual description 
			vr_key_event(void* _device_handle, unsigned _player_index, unsigned _controller_index, const vr::vr_kit_state& _state,
				unsigned short _key = 0, KeyAction _action = KA_PRESS, unsigned char _char = 0,
				unsigned char _modifiers = 0, double _time = 0);
			/// return the device id, by default returns 0
			void* get_device_handle() const { return device_handle; }
			/// return controller index (0 .. left, 1.. right) of vr kit
			unsigned get_controller_index() const { return controller_index; }
			/// return player index
			unsigned get_player_index() const { return player_index; }
			/// return the state
			const vr::vr_kit_state& get_state() const { return state; }
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
		};

		/// vr extension of throttle event
		class CGV_API vr_throttle_event : public throttle_event
		{
		protected:
			/// store device handle
			void* device_handle;
			/// access to current vr state 
			const vr::vr_kit_state& state;
		public:
			/// construct a throttle event from value and value change
			vr_throttle_event(void* _device_handle, unsigned _controller_index, const vr::vr_kit_state& _state,
				float _x, float _dx, unsigned _player_index = 0, unsigned _throttle_index = 0, double _time = 0);
			/// return the device id, by default returns 0
			void* get_device_handle() const { return device_handle; }
			/// return the state
			const vr::vr_kit_state& get_state() const { return state; }
			/// write to stream
			void stream_out(std::ostream& os) const;
		};

		/// vr extension of stick event
		class CGV_API vr_stick_event : public stick_event
		{
		protected:
			/// store device handle
			void* device_handle;
			/// access to current vr state 
			const vr::vr_kit_state& state;
		public:
			/// construct a key event from its textual description 
			vr_stick_event(void* _device_handle, unsigned _controller_index, const vr::vr_kit_state& _state,
				StickAction _action, float _x, float _y, float _dx, float _dy,
				unsigned _player_index = 0, unsigned _stick_index = 0, double _time = 0);
			/// return the device id, by default returns 0
			void* get_device_handle() const { return device_handle; }
			/// return the state
			const vr::vr_kit_state& get_state() const { return state; }
			/// write to stream
			void stream_out(std::ostream& os) const;
		};

		/// vr extension of pose events
		class CGV_API vr_pose_event : public pose_event
		{
		protected:
			/// store device handle
			void* device_handle;
			/// access to current vr state 
			const vr::vr_kit_state& state;
		public:
			/// construct a key event from its textual description 
			vr_pose_event(void* _device_handle, short _trackable_index, const vr::vr_kit_state& _state,
				const float *_pose, const float *_last_pose, unsigned short _player_index, double _time = 0);
			/// return the device id, by default returns 0
			void* get_device_handle() const { return device_handle; }
			/// return the state
			const vr::vr_kit_state& get_state() const { return state; }
			/// write to stream
			void stream_out(std::ostream& os) const;
		};

	}
}

///@}

#include <cgv/config/lib_end.h>
