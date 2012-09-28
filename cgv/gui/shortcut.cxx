#include "shortcut.h"
#include "event.h"
#include <cgv/utils/scan.h>

using namespace cgv::utils;

namespace cgv {
	namespace gui {

const char* key_strings[] = {
	"F1", 
	"F2", 
	"F3", 
	"F4", 
	"F5", 
	"F6", 
	"F7", 
	"F8", 
	"F9", 
	"F10", 
	"F11", 
	"F12",

	"Space", 
	"Enter", 
	"Tab", 
	"Print",
	"Pause",
	"Break", 
	"Escape", 

	"Left_Shift", 
	"Right_Shift", 
	"Left_Alt", 
	"Right_Alt", 
	"Left_Ctrl", 
	"Right_Ctrl", 
	"Left_Meta", 
	"Right_Meta",

	"Left", 
	"Right", 
	"Up", 
	"Down", 
	"Home", 
	"End", 
	"Page_Up", 
	"Page_Down",

	"Back_Space", 
	"Delete", 
	"Insert",

	"Caps_Lock",
	"Num_Lock",
	"Scroll_Lock",

	"Num_0",
	"Num_1",
	"Num_2",
	"Num_3",
	"Num_4",
	"Num_5",
	"Num_6",
	"Num_7",
	"Num_8",
	"Num_9",
	"Num_Div",
	"Num_Mul",
	"Num_Sub",
	"Num_Add",
	"Num_Dot",
	"Num_Enter",
};

std::string get_key_string(unsigned short key)
{
	if (key > 255)
		return key_strings[key-256];
	else {
		if (key == 0)
			return "";
		return std::string(1, (unsigned char) key);
	}
}

shortcut::shortcut(unsigned short _key, unsigned char _modifiers) : key(_key), modifiers(_modifiers) 
{
}


/// ensure that in case of modifier key presses the modifier is not set as such
void shortcut::validate()
{
	if (key == KEY_Left_Shift || key == KEY_Right_Shift)
		modifiers &= ~EM_SHIFT;
	if (key == KEY_Left_Ctrl || key == KEY_Right_Ctrl)
		modifiers &= ~EM_CTRL;
	if (key == KEY_Left_Alt || key == KEY_Right_Alt)
		modifiers &= ~EM_ALT;
}

/// write to stream
void shortcut::stream_out(std::ostream& os) const
{
	os << get_modifier_string(modifiers) << get_key_string(key);
}

/// read from stream
bool shortcut::stream_in(std::istream& is)
{
	std::string token, read;
	unsigned char _modifiers = 0;
	unsigned short _key = 0;
	char last_sep = 0;
	while (!is.eof()) {
		char c;
		is.get(c);
		if (is.fail())
			break;
		bool do_not_extend_token = false;
		if (token.empty()) {
			switch (c) {
			case '#' :
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' :
			case '0' :
			case '+' :
			case '-' :
			case ',' :
			case '.' :
			case '^' :
			case '<' :
			case '\\' :
			case '´' :
			case ' ' :
				key = c;
				modifiers = _modifiers;
				return true;
			}
		}
		if (c == '-' || c == '+') {
			if (to_upper(token) == "SHIFT")
				_modifiers |= EM_SHIFT;
			else if (to_upper(token) == "ALT")
				_modifiers |= EM_ALT;
			else if (to_upper(token) == "CTRL")
				_modifiers |= EM_CTRL;
			else if (to_upper(token) == "META")
				_modifiers |= EM_META;
			else {
				is.putback(c);
				while (read.size() > 0) {
					is.putback(read[read.size()-1]);
					read = read.substr(0,read.size()-1);
				}
				return false;
			}
			do_not_extend_token = true;
			token.clear();
		}
		if (c == ' ' || c == ',' || c == ';') {
			last_sep = c;
			break;
		}
		if (!do_not_extend_token)
			token.push_back(c);

		read.push_back(c);
	}
	if (token.size() == 1) {
		key = token[0];
		modifiers = _modifiers;
		return true;
	}
	for (unsigned i = 256; i < KEY_Last; ++i) {
		if (to_upper(token) == to_upper(key_strings[i-256])) {
			key = i;
			modifiers = _modifiers;
			return true;
		}
	}
	if (last_sep)
		is.putback(last_sep);
	while (read.size() > 0) {
		is.putback(read[read.size()-1]);
		read = read.substr(0,read.size()-1);
	}
	return false;
}

/// stream a shortcut as text to an output stream
std::ostream& operator << (std::ostream& os, const shortcut& sc)
{
	sc.stream_out(os);
	return os;
}

/// stream in a shortcut from an input stream
std::istream& operator >> (std::istream& is, shortcut& sc)
{
	if (!sc.stream_in(is))
		is.setstate(std::ios::failbit);
	else
		is.clear();
	return is;
}

	}
}
