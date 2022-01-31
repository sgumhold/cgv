#pragma once

#include "lib_begin.h"

namespace cgv {
	namespace os {


/// reposition mouse cursor on desktop
extern CGV_API void set_mouse_cursor(int x, int y);

/// read the mouse cursor position into coordinate references and return whether this was successful
extern CGV_API bool get_mouse_cursor(int& x, int& y);

/// send a mouse button press or release event for button 0 .. left, 1 .. middle, 2 .. right
extern CGV_API void send_mouse_button_event(int button, bool press_not_release);

/// send a mouse wheel event where dy == 1 corresponds to a single tick of the wheel (fractional values are possible)
extern CGV_API void send_mouse_wheel_event(float dy);

	}
}

#include <cgv/config/lib_end.h>
