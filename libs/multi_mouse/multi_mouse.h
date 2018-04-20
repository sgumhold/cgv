#pragma once

#include <vector>
#include <cgv/gui/mouse_event.h>
#include "lib_begin.h"

namespace cgv {
	namespace gui {

/// structure with information about a mouse device
struct CGV_API mouse_info
{
	/// mouse id that is used in the global interface and stored in the multi_mouse_event
	void* device_id;
	/// return the number of mouse buttons
	unsigned nr_buttons;
	/// whether the mouse has a horizontal wheel
	bool horizontal_wheel;
};

/// extend mouse event by id of mouse
class CGV_API multi_mouse_event : public mouse_event
{
protected:
	/// store id of mouse
	void* device_id;
public:
	/// construct a mouse
	multi_mouse_event(int x = 0, int y = 0, MouseAction _action = MA_MOVE, unsigned char _button_state = 0, unsigned char _button = 0, short _dx = 0, short _dy = 0, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
	/// write to stream
	void stream_out(std::ostream& os) const;
	/// read from stream
	void stream_in(std::istream& is);
	/// return id of mouse
	void* get_device_id() const;
	/// set the id
	void set_device_id(void* _id);
};

/// handler interface for multi mouse events
struct CGV_API multi_mouse_handler
{
	/// callback for handling event. Return whether this event has been processed
	virtual bool handle_multi_mouse(const multi_mouse_event& mme);
	/// called when a new mouse device is attached (attach=true) or an existing is detached (attach=false). Detach does not work!!!
	virtual void on_mouse_device_change(bool attach, void* device_id);
};

/// turn on multi mouse support, either disabling standard mouse functionality or not
extern CGV_API bool enable_multi_mouse(bool disable_mouse = false);

/// register a multi mouse event handler 
extern CGV_API void register_multi_mouse_handler(multi_mouse_handler* handler);

/// unregister a multi mouse event handler 
extern CGV_API void unregister_multi_mouse_handler(multi_mouse_handler* handler);

/// scan all mouse devices
extern CGV_API bool scan_mouse_devices(std::vector<mouse_info>& mis);

/// return the location of the pointer for the given mouse
extern CGV_API void get_mouse_position(void* mouse_id, int& x, int& y);

/// return the button state of the given mouse
extern CGV_API unsigned get_mouse_button_state(void* mouse_id);

/// return the rectangle to which the mouse motion is constraint
extern CGV_API void get_mouse_rectangle(void* mouse_id, int& xmin, int& xmax, int& ymin, int& ymax);

/// set the location of the pointer for the given mouse
extern CGV_API void set_mouse_position(void* mouse_id, int x, int y);

/// set the rectangle to which the mouse motion is constraint
extern CGV_API void set_mouse_rectangle(void* mouse_id, int xmin, int xmax, int ymin, int ymax);

	}
}

#include <cgv/config/lib_end.h>
