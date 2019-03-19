#include "key_event.h"
#include "shortcut.h"

namespace cgv {
	namespace gui {


/// convert a key action into a readable string
std::string get_key_action_string(KeyAction action)
{
	switch (action) {
	case KA_PRESS: return "press";
	case KA_RELEASE: return "release";
	case KA_REPEAT: return "repeat";
	default: return "unknown";
	}
}

// construct a key event from its textual description 
key_event::key_event(unsigned short _key, KeyAction _action, unsigned char _char, unsigned char _modifiers, unsigned char _toggle_keys, double _time) 
	: event(EID_KEY,_modifiers, _toggle_keys, _time), key(_key), action(_action), character(_char)
{
}

// write to stream
void key_event::stream_out(std::ostream& os) const
{
	event::stream_out(os);
	os << get_key_string(key).c_str();
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
}

// read from stream
void key_event::stream_in(std::istream&)
{
	std::cerr << "key_event::stream_in not implemented yet" << std::endl;
}

// 
unsigned short key_event::get_key() const
{
	return key;
}
// 
void key_event::set_key(unsigned short _key)
{
	key = _key;
}
// 
unsigned char key_event::get_char() const
{
	return character;
}
// 
void key_event::set_char(unsigned char _char)
{
	character = _char;
}
// 
KeyAction key_event::get_action() const
{
	return (KeyAction)action;
}
// 
void key_event::set_action(KeyAction _action)
{
	action = _action;
}

	}
}
