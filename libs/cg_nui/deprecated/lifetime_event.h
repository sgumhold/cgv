#pragma once

#include "nui_event.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		/// different lifetime event types
		enum LifetimeEventType
		{
			LET_BIRTH,
			LET_EVOLVE,
			LET_DEATH
		};

		/// convert an lifetime event type into a readable string
		extern CGV_API std::string get_lifetime_type_string(LifetimeEventType type);

		/// class to represent lifetime events that include birth, time evolution and death
		class CGV_API lifetime_event : public nui_event
		{
		protected:
			/// store type of lifetime event
			LifetimeEventType type;
			/// time difference since last event, used for evolve events only
			float dt;
		public:
			/// construct birth or death event
			lifetime_event(bool birth, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
			/// construct evolve event
			lifetime_event(float _dt, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
			/// return the type of the lifetime event
			LifetimeEventType get_type() const;
			/// set the type of the lifetime event
			void set_type(LifetimeEventType _type);
			/// return time difference
			float get_dt() const;
			/// set time difference
			void set_dt(float _dt);
		};
	}
}