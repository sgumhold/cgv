#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "mouse_win.h"

namespace trajectory {
namespace util {

	bool mouse::set_position(int x, int y) { return SetCursorPos(x, y); }

	void mouse::hold_left()
	{
		INPUT inputs[1] = {0};
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
		SendInput(1, inputs, sizeof(INPUT));
	}

	void mouse::release_left()
	{
		INPUT inputs[1] = {0};
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;
		SendInput(1, inputs, sizeof(INPUT));
	}

	void mouse::left_click()
	{
		hold_left();
		release_left();
	}

	void mouse::double_left_click()
	{
		left_click();
		left_click();
	}

	void mouse::right_click()
	{
		INPUT inputs[2] = {0};
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;

		inputs[1].type = INPUT_MOUSE;
		inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;

		SendInput(2, inputs, sizeof(INPUT));
	}

	void mouse::hold_middle()
	{
		INPUT inputs[1] = {0};
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
		SendInput(1, inputs, sizeof(INPUT));
	}

	void mouse::release_middle()
	{
		INPUT inputs[1] = {0};
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
		SendInput(1, inputs, sizeof(INPUT));
	}

	void mouse::middle_click()
	{
		hold_middle();
		release_middle();
	}

	void mouse::scroll(int n)
	{
		INPUT inputs[1] = {0};
		inputs[0].type = INPUT_MOUSE;
		inputs[0].mi.dx = 0;
		inputs[0].mi.dy = 0;
		inputs[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
		inputs[0].mi.time = 0;
		inputs[0].mi.mouseData = n * WHEEL_DELTA;

		SendInput(1, inputs, sizeof(INPUT));
	}

	void mouse::scroll_up(unsigned int n) { scroll(n); }

	void mouse::scroll_down(unsigned int n) { scroll(-(int)n); }

} // namespace util
} // namespace trajectory