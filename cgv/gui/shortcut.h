#pragma once

#include <string>
#include <iostream>
#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// define the names of special keys
enum Keys {
	KEY_F1 = 256, //!< F1 key
	KEY_F2, //!< F2 key
	KEY_F3, //!< F3 key
	KEY_F4, //!< F4 key
	KEY_F5, //!< F5 key
	KEY_F6, //!< F6 key
	KEY_F7, //!< F7 key
	KEY_F8, //!< F8 key
	KEY_F9, //!< F9 key
	KEY_F10, //!< F10 key
	KEY_F11, //!< F11 key
	KEY_F12, //!< F12 key
	KEY_F13, //!< F13 key
	KEY_F14, //!< F14 key
	KEY_F15, //!< F15 key
	KEY_F16, //!< F16 key
	KEY_F17, //!< F17 key
	KEY_F18, //!< F18 key
	KEY_F19, //!< F19 key
	KEY_F20, //!< F20 key
	KEY_F21, //!< F21 key
	KEY_F22, //!< F22 key
	KEY_F23, //!< F23 key
	KEY_F24, //!< F24 key
	KEY_F25, //!< F25 key

	KEY_Space, //!< space key
	KEY_Enter, //!< enter key
	KEY_Tab, //!< tab key
	KEY_Print, //!< print key
	KEY_Pause, //!< pause key
	KEY_Break, //!< break key
	KEY_Escape, //!< escape key
	KEY_Menu, //!< menu key

	KEY_Left_Shift, //!< left shift key
	KEY_Right_Shift, //!< right shift key
	KEY_Left_Alt, //!< left alt key
	KEY_Right_Alt, //!< right alt key
	KEY_Left_Ctrl, //!< left ctrl key
	KEY_Right_Ctrl, //!< right ctrl key
	KEY_Left_Meta, //!< left meta key
	KEY_Right_Meta, //!< right meta key

	KEY_Left, //!< left arrow key
	KEY_Right, //!< right arrow key
	KEY_Up, //!< up arrow key
	KEY_Down, //!< down arrow key
	KEY_Home, //!< home key
	KEY_End, //!< end key
	KEY_Page_Up, //!< page up key
	KEY_Page_Down, //!< page down key

	KEY_Back_Space, //!< back space key 
	KEY_Delete, //!< delete key
	KEY_Insert, //!< insert key

	KEY_Caps_Lock, //!< caps lock key
	KEY_Num_Lock, //!< num lock key
	KEY_Scroll_Lock, //!< scroll lock key

	KEY_Num_0, //!< num pad key 0
	KEY_Num_1, //!< num pad key 1
	KEY_Num_2, //!< num pad key 2
	KEY_Num_3, //!< num pad key 3 
	KEY_Num_4, //!< num pad key 4
	KEY_Num_5, //!< num pad key 5
	KEY_Num_6, //!< num pad key 6
	KEY_Num_7, //!< num pad key 7
	KEY_Num_8, //!< num pad key 8
	KEY_Num_9, //!< num pad key 9
	KEY_Num_Div, //!< num pad key /
	KEY_Num_Mul, //!< num pad key *
	KEY_Num_Sub, //!< num pad key -
	KEY_Num_Add, //!< num pad key +
	KEY_Num_Dot, //!< num pad key .
	KEY_Num_Enter, //!< num pad enter key 

	KEY_Last
};

/// convert a key code into a readable string
extern CGV_API std::string get_key_string(unsigned short key);

/// the shortcut class encapsulates a key with modifiers
class CGV_API shortcut
{
protected:
	///
	unsigned short key;
	///
	unsigned char modifiers;
public:
	///
	shortcut(unsigned short _key = 0, unsigned char _mod = 0);
	/// write to stream
	void stream_out(std::ostream& os) const;
	/// read from stream
	bool stream_in(std::istream& is);
	/// return the key
	unsigned short get_key() const { return key; }
	/// return the modifier set
	unsigned char get_modifiers() const { return modifiers; }
	/// set the key
	void set_key(unsigned short _key) { key = _key; }
	/// set the modifier set
	void set_modifiers(unsigned char _modifiers) { modifiers = _modifiers; }
	/// ensure that in case of modifier key presses the modifier is not set as such
	void validate();
};

/// stream a shortcut as text to an output stream
extern CGV_API std::ostream& operator << (std::ostream& os, const shortcut& sc);

/// stream in a shortcut from an input stream
extern CGV_API std::istream& operator >> (std::istream& is, shortcut& sc);

	}
}

#include <cgv/config/lib_end.h>