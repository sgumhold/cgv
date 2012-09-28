#pragma once

#include <windows.h>


#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// process a raw input structure and send resulting events
extern CGV_API void process_raw_input(HANDLE id, const RAWMOUSE& ri);

/// process a raw input structure and send resulting events
extern CGV_API bool register_window_to_mouse_device_change(HWND hWnd);

/// define type of a pointer to a window procedure
typedef LRESULT (WINAPI *window_proc_pointer_type)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

//! return a pointer to a window procedure that handles WM_INPUT messages of mouse devices.
/*! The pointer in the argument to the function is another window procedure that
    is used by the returned window procedure of the message is not handled. */
extern CGV_API window_proc_pointer_type get_wnd_proc_link(window_proc_pointer_type def_wnd_proc = 0);

	}
}

#include <cgv/config/lib_end.h>
