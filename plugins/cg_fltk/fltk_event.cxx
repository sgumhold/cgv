#include "fltk_event.h"
#include <cgv/gui/mouse_event.h>
#include <cgv/gui/shortcut.h>
#include <cgv/utils/scan.h>
#include <fltk/events.h>
#include <fltk/run.h>

using namespace cgv::gui;

unsigned char cgv_modifiers(int fltk_state)
{
	unsigned char modifiers = 0;
	if (fltk_state & fltk::SHIFT) 
		modifiers += EM_SHIFT;
	if (fltk_state & fltk::ALT) 
		modifiers += EM_ALT;
	if (fltk_state & fltk::CTRL) 
		modifiers += EM_CTRL;
	if (fltk_state & fltk::META) 
		modifiers += EM_META;
	return modifiers;
}

unsigned char cgv_toggle_keys(int fltk_state)
{
	unsigned char t = 0;
	if (fltk_state & fltk::CAPSLOCK) 
		t += ETK_CAPS_LOCK;
	if (fltk_state & fltk::NUMLOCK) 
		t += ETK_NUM_LOCK;
	if (fltk_state & fltk::SCROLLLOCK) 
		t += ETK_SCROLL_LOCK;
	return t;
}

unsigned short* get_key_translation_table()
{
	static unsigned short key_translation_table[256] =
	{
		0,0,0,0,0,0,0,0,KEY_Back_Space,KEY_Tab,0,0,0,KEY_Enter,0,0,
		0,0,0,KEY_Pause,KEY_Scroll_Lock,0,0,0,0,0,0,KEY_Escape,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		KEY_Home, KEY_Left, KEY_Up, KEY_Right, KEY_Down, KEY_Page_Up, KEY_Page_Down, KEY_End, 0,0,0,0,0,0,0,0,
		0, KEY_Print, 0, KEY_Insert, 0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,KEY_Num_Lock,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,KEY_F1,KEY_F2,KEY_F3,
		KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,'A','B',
		KEY_Left_Shift,KEY_Right_Shift,KEY_Left_Ctrl,KEY_Right_Ctrl,KEY_Caps_Lock,0,KEY_Left_Meta,KEY_Right_Meta,KEY_Left_Alt,KEY_Right_Alt,'D','E','F','G','H',
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,KEY_Delete
	};
	static bool initialized = false;
	if (!initialized) {
		initialized = true;
		key_translation_table[fltk::KeypadEnter-0xff00] = KEY_Num_Enter;
		key_translation_table[fltk::MultiplyKey-0xff00] = KEY_Num_Mul;
		key_translation_table[fltk::AddKey-0xff00]      = KEY_Num_Add;
		key_translation_table[fltk::SubtractKey-0xff00] = KEY_Num_Sub;
		key_translation_table[fltk::DecimalKey-0xff00]  = KEY_Num_Dot;
		key_translation_table[fltk::DivideKey-0xff00]   = KEY_Num_Div;
		key_translation_table[fltk::Keypad0-0xff00] = KEY_Num_0;
		key_translation_table[fltk::Keypad1-0xff00] = KEY_Num_1;
		key_translation_table[fltk::Keypad2-0xff00] = KEY_Num_2;
		key_translation_table[fltk::Keypad3-0xff00] = KEY_Num_3;
		key_translation_table[fltk::Keypad4-0xff00] = KEY_Num_4;
		key_translation_table[fltk::Keypad5-0xff00] = KEY_Num_5;
		key_translation_table[fltk::Keypad6-0xff00] = KEY_Num_6;
		key_translation_table[fltk::Keypad7-0xff00] = KEY_Num_7;
		key_translation_table[fltk::Keypad8-0xff00] = KEY_Num_8;
		key_translation_table[fltk::Keypad9-0xff00] = KEY_Num_9;
	}
	return key_translation_table;
}

unsigned short cgv_key(int fltk_key)
{
	unsigned short key = fltk_key;
	if (key == 32)
		key = KEY_Space;
	else if(key >= 0xff00)
		key = get_key_translation_table()[fltk::event_key()-0xff00];
	else if (key < 256)
		key = cgv::utils::to_upper((unsigned char)key);
	return key;
}

/// translate fltk shortcut to cgv shortcut
cgv::gui::shortcut cgv_shortcut(int fltk_shortcut)
{
	cgv::gui::shortcut sc(cgv_key(fltk_shortcut & 0xffff), cgv_modifiers(fltk_shortcut));
	sc.validate();
	return sc;
}

int fltk_modifiers(unsigned char modifiers)
{
	int fltk_mods = 0;
	if (modifiers & EM_SHIFT)
		fltk_mods += fltk::SHIFT;
	if (modifiers & EM_ALT)
		fltk_mods += fltk::ALT;
	if (modifiers & EM_CTRL)
		fltk_mods += fltk::CTRL;
	if (modifiers & EM_META)
		fltk_mods += fltk::META;
	return fltk_mods;
}

/// extract cgv toggle key set from fltk event state
int fltk_toggle_keys(unsigned char cgv_toggle_keys)
{
	int fltk_togs = 0;
	if (cgv_toggle_keys & ETK_CAPS_LOCK)
		fltk_togs += fltk::CAPSLOCK;
	if (cgv_toggle_keys & ETK_NUM_LOCK)
		fltk_togs += fltk::NUMLOCK;
	if (cgv_toggle_keys & ETK_SCROLL_LOCK)
		fltk_togs += fltk::SCROLLLOCK;
	return fltk_togs;
}

/// define the names of special keys
int fltk_keys[] = {
	fltk::F1Key, 
	fltk::F2Key, 
	fltk::F3Key, 
	fltk::F4Key, 
	fltk::F5Key, 
	fltk::F6Key, 
	fltk::F7Key, 
	fltk::F8Key, 
	fltk::F9Key, 
	fltk::F10Key, 
	fltk::F11Key, 
	fltk::F12Key, 
	fltk::SpaceKey,
	fltk::ReturnKey,
	fltk::TabKey,
	fltk::PrintKey,
	fltk::PauseKey,
	fltk::PauseKey,
	fltk::EscapeKey,

  fltk::LeftShiftKey	,
  fltk::RightShiftKey	,
  fltk::LeftAltKey	,
  fltk::RightAltKey	,
  fltk::LeftCtrlKey	,
  fltk::RightCtrlKey	,
  fltk::LeftMetaKey	,
  fltk::RightMetaKey	,

  fltk::LeftKey	, 
  fltk::RightKey	, 
  fltk::UpKey		, 
  fltk::DownKey	, 
  fltk::HomeKey	, 
  fltk::EndKey	, 
  fltk::PageUpKey	, 
  fltk::PageDownKey	, 
  fltk::BackSpaceKey	, 
  fltk::DeleteKey	, 
  fltk::InsertKey	, 

  fltk::CapsLockKey	, 
  fltk::NumLockKey	, 
  fltk::ScrollLockKey	, 

  fltk::Keypad0	, 
  fltk::Keypad1	, 
  fltk::Keypad2	, 
  fltk::Keypad3	, 
  fltk::Keypad4	, 
  fltk::Keypad5	, 
  fltk::Keypad6	, 
  fltk::Keypad7	, 
  fltk::Keypad8	, 
  fltk::Keypad9	, 

  fltk::DivideKey	, 
  fltk::MultiplyKey	, 
  fltk::SubtractKey	, 
  fltk::AddKey	, 
  fltk::DecimalKey	, 
  fltk::KeypadEnter
};


int fltk_key(unsigned short cgv_key)
{
	if (cgv_key >= KEY_F1)
		return fltk_keys[cgv_key];
	return cgv::utils::to_lower((unsigned char)cgv_key);
}

int fltk_shortcut(const cgv::gui::shortcut& sc)
{
	return fltk_key(sc.get_key())+fltk_modifiers(sc.get_modifiers());
}

/*
/// translate a cgv key with modifiers into a fltk short cut
//int fltk_shortcut(unsigned short _key, unsigned char _modifiers)
int fltk_shortcut(cgv::gui::shortcut& sc)
{
	unsigned fltk_key;

	// translate key
	if (sc.get_key() == KEY_Space)
		fltk_key = 32;
	else if(sc.get_key() >= KEY_F1) {
		for (fltk_key = 0; fltk_key < 256; ++fltk_key) {
			if (get_key_translation_table()[fltk_key] == sc.get_key())
				break;
		}
		if (fltk_key == 256) {
			std::cerr << "could not translate sc.get_key() " << sc.get_key() << std::endl;
			fltk_key = 0;
		}
		else
			fltk_key += 0xff00;
	}
	else if (sc.get_key() < 256)
		fltk_key = cgv::utils::to_upper((unsigned char)sc.get_key());


	// translate modifiers
	if (sc.get_modifiers() & EM_SHIFT)
		fltk_key += fltk::SHIFT;
	if (sc.get_modifiers() & EM_ALT)
		fltk_key += fltk::ALT;
	if (sc.get_modifiers() & EM_CTRL)
		fltk_key += fltk::CTRL;
	if (sc.get_modifiers() & EM_META)
		fltk_key += fltk::META;

	return fltk_key;
}
*/
key_event cgv_key_event(KeyAction a)
{
	char c = 0;
	if (fltk::event_length() == 1)
		c = fltk::event_text()[0];

	unsigned short key = cgv_key(fltk::event_key());

	return key_event(key, a, c, cgv_modifiers(fltk::event_state()), cgv_toggle_keys(fltk::event_state()), fltk::get_time_secs(), fltk::event_x(), fltk::event_y());
}

cgv::gui::mouse_event cgv_mouse_event(MouseAction a, int dx, int dy)
{
	unsigned char bs = 0;
	if ((fltk::event_state()&fltk::BUTTON1)!=0)
		bs |= MB_LEFT_BUTTON;
	if ((fltk::event_state()&fltk::BUTTON2)!=0)
		bs |= MB_MIDDLE_BUTTON;
	if ((fltk::event_state()&fltk::BUTTON3)!=0)
		bs |= MB_RIGHT_BUTTON;

	unsigned char b = 0;
	if (a == MA_PRESS || a == MA_RELEASE) {
		if (fltk::event_button() == fltk::LeftButton)
			b = MB_LEFT_BUTTON;
		else if (fltk::event_button() == fltk::MiddleButton)
			b = MB_MIDDLE_BUTTON;
		else if (fltk::event_button() == fltk::RightButton)
			b = MB_RIGHT_BUTTON;
	}
	if (a == MA_WHEEL)
		dy = fltk::event_dy();
	return cgv::gui::mouse_event(fltk::event_x(), fltk::event_y(), a, bs, b, dx, dy, cgv_modifiers(fltk::event_state()), cgv_toggle_keys(fltk::event_state()), fltk::get_time_secs());
}

cgv::gui::mouse_event cgv_mouse_event(cgv::gui::MouseAction a, cgv::gui::EventFlags flags, int dx, int dy)
{
	cgv::gui::mouse_event me = cgv_mouse_event(a, dx, dy);
	me.set_flags(me.get_flags()|flags);
	return me;
}
