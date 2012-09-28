#include "multi_mouse_detail.h"
#include "multi_mouse_win32.h"

namespace cgv {
	namespace gui {

void process_raw_input(void* id, const RAWMOUSE& ri)
{
	mouse_pointer_info& mpi = ref_mouse_pointer_map()[id];

	// clip movement at rectangle
	int dx = ri.lLastX, dy = ri.lLastY;
	if (mpi.x + dx < mpi.xmin)
		dx = mpi.xmin - mpi.x;
	if (mpi.x + dx >= mpi.xmax)
		dx = mpi.xmax - mpi.x - 1;
	if (mpi.y + dy < mpi.ymin)
		dy = mpi.ymin - mpi.y;
	if (mpi.y + dy >= mpi.ymax)
		dy = mpi.ymax - mpi.y - 1;

	// prepare event
	mpi.x += dx;
	mpi.y += dy;
	multi_mouse_event mme(mpi.x, mpi.y);
	mme.set_id(id);
	mme.set_flags(EF_MULTI);
	mme.set_button_state(mpi.button_state);
	mme.set_dx((short)dx);
	mme.set_dy((short)dy);

	// find out about modifiers
	unsigned char mods = 0;
	if (GetAsyncKeyState(VK_SHIFT) > 1)
		mods |= EM_SHIFT;
	if (GetAsyncKeyState(VK_CONTROL) > 1)
		mods |= EM_CTRL;
	if (GetAsyncKeyState(VK_MENU) > 1)
		mods |= EM_ALT;
	if ((GetAsyncKeyState(VK_LWIN) > 1) || (GetAsyncKeyState(VK_RWIN) > 1))
		mods |= EM_META;
	mme.set_modifiers(mods);

	// find out about toggle keys
	unsigned char toggles = 0;
	if (GetAsyncKeyState(VK_CAPITAL) & 1)
		toggles |= ETK_CAPS_LOCK;
	if (GetAsyncKeyState(VK_NUMLOCK) & 1)
		toggles |= ETK_NUM_LOCK;
	if (GetAsyncKeyState(VK_SCROLL) & 1)
		toggles |= ETK_SCROLL_LOCK;
	mme.set_toggle_keys(toggles);

	// send move or drag event if necessary
	if (dx != 0 || dy != 0) {
		if (mme.get_button_state() != 0)
			mme.set_action(MA_DRAG);
		else
			mme.set_action(MA_MOVE);
		emit_multi_mouse_event(mme);
	}
	mme.set_dx(0);
	mme.set_dy(0);

	if (ri.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
		mme.set_action(MA_PRESS);
		mme.set_button(MB_LEFT_BUTTON);
		emit_multi_mouse_event(mme);
		mme.set_button_state(mpi.button_state |= MB_LEFT_BUTTON);
	}
	if (ri.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
		mme.set_button_state(mpi.button_state &= ~MB_LEFT_BUTTON);
		mme.set_action(MA_RELEASE);
		mme.set_button(MB_LEFT_BUTTON);
		emit_multi_mouse_event(mme);
	}
	if (ri.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
		mme.set_action(MA_PRESS);
		mme.set_button(MB_MIDDLE_BUTTON);
		emit_multi_mouse_event(mme);
		mme.set_button_state(mpi.button_state |= MB_MIDDLE_BUTTON);
	}
	if (ri.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP) {
		mme.set_button_state(mpi.button_state &= ~MB_MIDDLE_BUTTON);
		mme.set_action(MA_RELEASE);
		mme.set_button(MB_MIDDLE_BUTTON);
		emit_multi_mouse_event(mme);
	}
	if (ri.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
		mme.set_action(MA_PRESS);
		mme.set_button(MB_RIGHT_BUTTON);
		emit_multi_mouse_event(mme);
		mme.set_button_state(mpi.button_state |= MB_RIGHT_BUTTON);
	}
	if (ri.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
		mme.set_button_state(mpi.button_state &= ~MB_RIGHT_BUTTON);
		mme.set_action(MA_RELEASE);
		mme.set_button(MB_RIGHT_BUTTON);
		emit_multi_mouse_event(mme);
	}
	if (ri.usButtonFlags & RI_MOUSE_WHEEL) {
		mme.set_action(MA_WHEEL);
		mme.set_button(MB_NO_BUTTON);
		mme.set_dx(ri.usButtonData);
		emit_multi_mouse_event(mme);
	}
}

/// process a raw input structure and send resulting events
bool register_window_to_mouse_device_change(HWND hWnd)
{
	RAWINPUTDEVICE Rid;
	        
	Rid.usUsagePage = 0x01; 
	Rid.usUsage = 0x02; 
	Rid.dwFlags = RIDEV_DEVNOTIFY;
	Rid.hwndTarget = hWnd;

	return RegisterRawInputDevices(&Rid,1, sizeof(Rid)) != FALSE;
}


static window_proc_pointer_type def_window_proc;

static LRESULT CALLBACK multi_mouse_window_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg != WM_INPUT) {
		if (uMsg != WM_INPUT_DEVICE_CHANGE)
			return def_window_proc(hWnd, uMsg, wParam, lParam);
		RID_DEVICE_INFO info;
		UINT size = sizeof(info);
		GetRawInputDeviceInfo((HANDLE&)lParam,RIDI_DEVICEINFO,&info,&size);
		if (info.dwType != RIM_TYPEMOUSE)
			return def_window_proc(hWnd, uMsg, wParam, lParam);
		emit_multi_mouse_device_change_event(wParam == GIDC_ARRIVAL, (void*&)lParam);
		return def_window_proc(hWnd, uMsg, wParam, lParam);
	}
/*
	static bool have_registered = false;
	if (!have_registered) {
		have_registered= true;
		RAWINPUTDEVICE Rid;
		        
		Rid.usUsagePage = 0x01; 
		Rid.usUsage = 0x02; 
		Rid.dwFlags = RIDEV_DEVNOTIFY;
		Rid.hwndTarget = hWnd;

		std::cout << RegisterRawInputDevices(&Rid,1, sizeof(Rid)) << std::endl;		
	}
	*/
	RAWINPUTHEADER header;
	UINT dwSize;
	GetRawInputData((HRAWINPUT)lParam, RID_HEADER, &header, &dwSize, sizeof(RAWINPUTHEADER));
	if (header.dwType != RIM_TYPEMOUSE)
		return def_window_proc(hWnd, uMsg, wParam, lParam);

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
	LPBYTE lpb = new BYTE[dwSize];
	if (lpb == NULL) 
		return def_window_proc(hWnd, uMsg, wParam, lParam);

	if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
		std::cerr << "GetRawInputData does not return correct size !\n"  << std::endl; 
	RAWINPUT* rip = (RAWINPUT*)lpb;
	process_raw_input(header.hDevice, rip->data.mouse);
	delete[] lpb; 
	return def_window_proc(hWnd, uMsg, wParam, lParam);
}

window_proc_pointer_type get_wnd_proc_link(window_proc_pointer_type _def_wnd_proc)
{
	def_window_proc = _def_wnd_proc;
	return multi_mouse_window_proc;
}


	}
}