#pragma once

#include "event.h"
#include "shortcut.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// different actions that a key can perform
enum KeyAction { 
	KA_RELEASE, //!< key release action
	KA_PRESS, //!< key press action
	KA_REPEAT //!< key repeated press action
};

/// class to represent all possible keyboard events with the EID_KEY
class CGV_API key_event : public event
{
protected:
	/// store the pressed key
	unsigned short key;
	/// store whether 
	unsigned char action;
	/// store the corresponding ascii character
	unsigned char character;
public:
	/// construct a key event from its textual description 
	key_event(unsigned short _key = 0, KeyAction _action = KA_PRESS, unsigned char _char = 0, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
	/// write to stream
	void stream_out(std::ostream& os) const;
	/// read from stream
	void stream_in(std::istream& is);
	/// return the key being a capital letter, digit or a value from the Keys enum
	unsigned short get_key() const;
	/// set the key
	void set_key(unsigned short _key);
	/// return the key %event action
	KeyAction get_action() const;
	/// set the key %event action
	void set_action(KeyAction _action);
	/// return the key as a character
	unsigned char get_char() const;
	/// set the alpha numeric character
	void set_char(unsigned char _char);
};

	}
}

#include <cgv/config/lib_end.h>