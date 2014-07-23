#pragma once
#include <cgv/gui/key_event.h>
#include <cgv/gui/mouse_event.h>

#include "lib_begin.h"

/// extract cgv modifier set from fltk event state
extern CGV_API unsigned char cgv_modifiers(int fltk_state);
/// translate a cgv modifier set into a fltk modifier set being part of a fltk event state
extern CGV_API int fltk_modifiers(unsigned char cgv_modifiers);

/// extract cgv toggle key set from fltk event state
extern CGV_API unsigned char cgv_toggle_keys(int fltk_state);
/// extract cgv toggle key set from fltk event state
extern CGV_API int fltk_toggle_keys(unsigned char cgv_toggle_keys);


/// translate a fltk key to a cgv key
extern CGV_API unsigned short cgv_key(int fltk_key);
/// translate a cgv key into a fltk key
extern CGV_API int fltk_key(unsigned short cgv_key);

/// translate fltk shortcut to cgv shortcut
extern CGV_API cgv::gui::shortcut cgv_shortcut(int fltk_shortcut);
/// translate a cgv shortcut into a fltk shortcut
extern CGV_API int fltk_shortcut(const cgv::gui::shortcut& sc);

/// convert the current fltk event into a cgv key event
extern CGV_API cgv::gui::key_event cgv_key_event(cgv::gui::KeyAction a);

/// convert the current fltk event into a cgv mouse event
extern CGV_API cgv::gui::mouse_event cgv_mouse_event(cgv::gui::MouseAction a, int dx = 0, int dy = 0);
extern CGV_API cgv::gui::mouse_event cgv_mouse_event(cgv::gui::MouseAction a, cgv::gui::EventFlags flags, int dx = 0, int dy = 0);

#include <cgv/config/lib_end.h>
