#pragma once

#include "nui_event.h"

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		/// different action event types
		enum ActionEventType
		{
			AET_GRAB,
			AET_DROP,
			AET_MOVE,
			AET_THROW
			// TODO: support, AET_COLLIDE events
		};

		/// convert an action event type into a readable string
		extern CGV_API std::string get_action_type_string(ActionEventType type);

		/// class to represent action events that include grabbing, moving and collisions
		class CGV_API action_event : public nui_event
		{
		protected:
			/// store type of action event
			ActionEventType type;
			/// point where object is picked
			vec3 pick_point;
			/// time difference for motion events
			float dt;
			/// rotation for motion events and angular velocity for throwing
			union {
				quat rotation;
				vec3 angular_velocity;
			};
			/// translation for motion events and linear velocity for throwing
			union {
				vec3 translation;
				vec3 linear_velocity;
			};
		public:
			/// construct grab or drop event
			action_event(bool grab, const vec3& pick_point, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
			/// construct move event
			action_event(const vec3& pick_point, float _dt, const quat& _rotation, const vec3& _translation, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
			/// construct throw
			action_event(const vec3& pick_point, const vec3& _angular_velocity, const vec3& _linear_velocity, const quat& _rotation, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
			/// write to stream
			void stream_out(std::ostream& os) const;
			/// read from stream
			void stream_in(std::istream& is);
			/// return the type of the action event
			ActionEventType get_type() const;
			/// set the type of the action event
			void set_type(ActionEventType _type);
			// TODO: add getter and setters for geometry
		};
	}
}