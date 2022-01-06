#pragma once

#include "nui_event.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		/// different approach event types
		enum ApproachEventType
		{
			AET_OVER,
			AET_EVADE, // opposite of over, i.e. something like "end of over"
			AET_TOUCH,
			AET_UNTOUCH, 
			AET_PRESS,
			AET_RELEASE
			// TODO: support, AET_COLLIDE events
		};

		/// convert an approach event type into a readable string
		extern CGV_API std::string get_approach_type_string(ApproachEventType type);

		/// class to represent approach events that include over, touch, press and their opposite
		class CGV_API approach_event : public nui_event
		{
		protected:
			/// store type of approach event
			ApproachEventType type;
			/// point where object is approached
			vec3 contact_point;
		public:
			/// construct grab or drop event
			approach_event(ApproachEventType _type, const vec3& _contact_point, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
			/// return the type of the approach event
			ApproachEventType get_type() const;
			/// set the type of the approach event
			void set_type(ApproachEventType _type);
			/// return contact point
			const vec3& get_contact_point() const;
			/// set new contact point
			void set_contact_point(const vec3& _contact_point);
		};
	}
}