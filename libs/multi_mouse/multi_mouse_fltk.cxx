#include "multi_mouse_fltk.h"
#include "multi_mouse_win32.h"
#include <fltk/compat/FL/Fl_Window.h>
#include <fltk/win32.h>

namespace cgv {
	namespace gui {

/// redirect fltk default windows procedure to process multi mouse messages
void attach_multi_mouse_to_fltk(fltk::Window* w)
{
	fltk::refDefWindowProc() = get_wnd_proc_link(fltk::refDefWindowProc());
	if (w)
		register_window_to_mouse_device_change(fltk::xid(w));
		
}

	}
}