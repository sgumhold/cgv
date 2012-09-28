#pragma once

#include "multi_mouse.h"
#include <map>

#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// structure that defines the information stored per mouse device
struct CGV_API mouse_pointer_info
{
	/// current location of device pointer
	int x, y;
	/// axes aligned bounding rectangle of valid device pointer locations
	int xmin, xmax, ymin, ymax;
	/// current state of device buttons
	unsigned char button_state;
	/// default constructor of mouse pointer info structure
	mouse_pointer_info();
};

/// declare type of map from device ids to mouse pointer info structures
typedef std::map<void*, mouse_pointer_info> map_type;

/// return a reference to a map from device ids to mouse pointer info structures
extern CGV_API map_type& ref_mouse_pointer_map();

/// used in windows procedures to emit multi mouse events to the registered multi_mouse_handlers
extern CGV_API void emit_multi_mouse_event(const multi_mouse_event& mme);

/// used to notify about change in device attachment
extern CGV_API void emit_multi_mouse_device_change_event(bool attach, void* id);
	}
}

#include <cgv/config/lib_end.h>
