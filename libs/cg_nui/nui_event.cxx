#include "nui_event.h"

namespace cgv {
	namespace nui {

/// construct an action event
nui_event::nui_event(unsigned _kind, unsigned char _modifiers, unsigned char _toggle_keys, double _time) : 
	gui::event(_kind, _modifiers, _toggle_keys, _time)
{
	flags |= gui::EF_NUI;
}

	}
}