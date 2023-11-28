#include "mouse_ctrl.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <iostream>
#endif

namespace cgv {
	namespace os {

#ifdef WIN32
void set_mouse_cursor(int x, int y)
{
	SetCursorPos(x, y);
}
bool get_mouse_cursor(int& x, int& y)
{
	POINT p;
	if (GetCursorPos(&p) == TRUE) {
		x = p.x;
		y = p.y;
		return true;
	}
	return false;
}

void send_mouse_button_event(int button, bool press_not_release)
{
	static const DWORD mouse_button_flag[6] = { 
		MOUSEEVENTF_LEFTUP, MOUSEEVENTF_MIDDLEUP, MOUSEEVENTF_RIGHTUP,
		MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_RIGHTDOWN
	};
	INPUT inputs[1] = { 0 };
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = mouse_button_flag[button + press_not_release?3:0];
	SendInput(1, inputs, sizeof(INPUT));
}
void send_mouse_wheel_event(float dy)
{
	INPUT inputs[1] = { 0 };
	inputs[0].type = INPUT_MOUSE;
	inputs[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
	inputs[0].mi.mouseData = DWORD(120 * dy);
	SendInput(1, inputs, sizeof(INPUT));
}
#else
/// reposition mouse cursor on desktop
void set_mouse_cursor(int x, int y)
{
	std::cerr << "cgv::os::set_mouse_cursor(x,y) not implemented on this platform" << std::endl;
}
bool get_mouse_cursor(int& x, int& y)
{
	std::cerr << "cgv::os::get_mouse_cursor(x, y) not implemented on this platform" << std::endl;
	return false;
}
void send_mouse_button_event(int button, bool press_not_release)
{
	std::cerr << "cgv::os::send_mouse_button_event(button,press_not_release) not implemented on this platform" << std::endl;
}
void send_mouse_wheel_event(float dy)
{
	std::cerr << "cgv::os::send_mouse_wheel_event(dy) not implemented on this platform" << std::endl;
}
#endif

	}
}