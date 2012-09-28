#include "multi_mouse_detail.h"

#include <algorithm>

namespace cgv {
	namespace gui {

mouse_pointer_info::mouse_pointer_info()
{
	x=y=xmin=ymin=0;
	xmax=ymax=1024;
	button_state = 0;
}

map_type& ref_mouse_pointer_map()
{
	static map_type map;
	return map;
}

/// return the location of the pointer for the given mouse
void get_mouse_position(void* mouse_id, int& x, int& y)
{
	mouse_pointer_info& info = ref_mouse_pointer_map()[mouse_id];
	x = info.x;
	y = info.y;
}

/// return the button state of the given mouse
unsigned get_mouse_button_state(void* mouse_id)
{
	return ref_mouse_pointer_map()[mouse_id].button_state;
}

/// return the rectangle to which the mouse motion is constraint
void get_mouse_rectangle(void* mouse_id, int& xmin, int& xmax, int& ymin, int& ymax)
{
	mouse_pointer_info& info = ref_mouse_pointer_map()[mouse_id];
	xmin = info.xmin;
	xmax = info.xmax;
	ymin = info.ymin;
	ymax = info.ymax;
}

/// set the location of the pointer for the given mouse
void set_mouse_position(void* mouse_id, int x, int y)
{
	mouse_pointer_info& info = ref_mouse_pointer_map()[mouse_id];
	info.x = x;
	info.y = y;
}

/// set the rectangle to which the mouse motion is constraint
void set_mouse_rectangle(void* mouse_id, int xmin, int xmax, int ymin, int ymax)
{
	mouse_pointer_info& info = ref_mouse_pointer_map()[mouse_id];
	info.xmin = xmin;
	info.xmax = xmax;
	info.ymin = ymin;
	info.ymax = ymax;
}

/// construct a mouse
multi_mouse_event::multi_mouse_event(int x, int y, MouseAction _action, unsigned char _button_state, unsigned char _button, 
									 short _dx, short _dy, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
									 : mouse_event(x,y,_action,_button_state,_button,_dx,_dy,_modifiers,_toggle_keys,_time)
{
	id = 0;
}
/// write to stream
void multi_mouse_event::stream_out(std::ostream& os) const
{
	mouse_event::stream_out(os);
	os << "{" << id << "}";
}

/// read from stream
void multi_mouse_event::stream_in(std::istream& is)
{
	mouse_event::stream_in(is);
	is.get();
	is >> id;
	is.get();
}

/// return id of mouse
void* multi_mouse_event::get_id() const
{
	return id;
}

/// set the id
void multi_mouse_event::set_id(void* _id)
{
	id = _id;
}

std::vector<multi_mouse_handler*>& ref_multi_mouse_handlers()
{
	static std::vector<multi_mouse_handler*> handlers;
	return handlers;
}

/// register a multi mouse event handler 
void register_multi_mouse_handler(multi_mouse_handler* handler)
{
	ref_multi_mouse_handlers().push_back(handler);
}

/// unregister a multi mouse event handler 
void unregister_multi_mouse_handler(multi_mouse_handler* h)
{
	std::vector<multi_mouse_handler*>& H = ref_multi_mouse_handlers();
	while (std::find(H.begin(),H.end(),h) != H.end()) {
		H.erase(std::find(H.begin(),H.end(),h));
	}
}

void emit_multi_mouse_event(const multi_mouse_event& mme)
{
	for (unsigned i=0; i<ref_multi_mouse_handlers().size(); ++i)
		ref_multi_mouse_handlers()[i]->handle_multi_mouse(mme);
}

void emit_multi_mouse_device_change_event(bool attach, void* id)
{
	for (unsigned i=0; i<ref_multi_mouse_handlers().size(); ++i)
		ref_multi_mouse_handlers()[i]->on_mouse_device_change(attach,id);
}


	}
}

#include <windows.h>

namespace cgv {
	namespace gui {

/// callback for handling event. Return whether this event has been processed
bool multi_mouse_handler::handle_multi_mouse(const multi_mouse_event& mme)
{
	return false;
}
/// called when a new mouse device is attached
void multi_mouse_handler::on_mouse_device_change(bool,void*)
{
}

/// turn on multi mouse support, either disabling standard mouse functionality or not
bool enable_multi_mouse(bool disable_mouse)
{
	RAWINPUTDEVICE Rid;
	        
	Rid.usUsagePage = 0x01; 
	Rid.usUsage = 0x02; 
	Rid.dwFlags = disable_mouse ? RIDEV_NOLEGACY : 0;
	Rid.hwndTarget = 0;

	return RegisterRawInputDevices(&Rid,1, sizeof(Rid)) != FALSE;
}

/// scan all mouse devices
bool scan_mouse_devices(std::vector<mouse_info>& mis)
{
	unsigned nr;
	if (GetRawInputDeviceList(NULL,&nr,sizeof(RAWINPUTDEVICELIST)) != 0)
		return false;

	RAWINPUTDEVICELIST* device_list = new RAWINPUTDEVICELIST[nr];
	nr = GetRawInputDeviceList(device_list,&nr,sizeof(RAWINPUTDEVICELIST));
	for (unsigned int i=0; i<nr; ++i) {
		wchar_t buffer[4096];
		unsigned size = 4096;
		GetRawInputDeviceInfo(device_list[i].hDevice,RIDI_DEVICENAME,buffer,&size);
		std::string type;
		if (device_list[i].dwType == RIM_TYPEMOUSE) {
			mis.push_back(mouse_info());
			mis.back().id = device_list[i].hDevice;

			RID_DEVICE_INFO info;
			info.cbSize = size = sizeof(RID_DEVICE_INFO);
			GetRawInputDeviceInfo(device_list[i].hDevice,RIDI_DEVICEINFO,&info,&size);
			mis.back().horizontal_wheel = info.mouse.fHasHorizontalWheel == TRUE;
			mis.back().nr_buttons = info.mouse.dwNumberOfButtons;
		}
	}
	return true;
}

	}
}