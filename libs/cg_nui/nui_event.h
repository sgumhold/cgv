#pragma once

#include <cgv/gui/event.h>
#include <cgv/render/render_types.h>

#include "lib_begin.h"

namespace cgv {
	namespace nui {

		/// base class to all nui events, which are marked with the EF_NUI event flag and have the render types declared
		class CGV_API nui_event : public gui::event, public render::render_types
		{
		public:
			/// construct a nui event
			nui_event(unsigned _kind, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
		};
	}
}

#include <cgv/config/lib_end.h>