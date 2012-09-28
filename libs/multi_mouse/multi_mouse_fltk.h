#pragma once

namespace fltk {
	class Window;
}

#include "lib_begin_fltk.h"

namespace cgv {
	namespace gui {

/// redirect fltk default windows procedure to process multi mouse messages. The passed window is used to register device change messages.
extern CGV_API void attach_multi_mouse_to_fltk(fltk::Window* w = 0);

	}
}

#include <cgv/config/lib_end.h>
