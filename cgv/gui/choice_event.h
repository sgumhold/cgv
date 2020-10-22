#pragma once

#include "event.h"
#include "shortcut.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

		/// different choice event types
		enum ChoiceEventType
		{
			CET_GRAB_FOCUS,
			CET_LOOSE_FOCUS,
			CET_SELECTED,
			CET_UNSELECTED
		};

		/// convert a choice event type into a readable string
		extern CGV_API std::string get_choice_type_string(ChoiceEventType type);

		/// class to represent choice events that include focus change and selection change events
		class CGV_API choice_event : public event
		{
		protected:
			/// store type of choice event
			ChoiceEventType type;
		public:
			/// construct a choice
			choice_event(ChoiceEventType _type, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
			/// return whether type of choice event
			ChoiceEventType get_type() const;
			/// set the type of the choice event
			void set_type(ChoiceEventType _type);
		};

	}
}

#include <cgv/config/lib_end.h>