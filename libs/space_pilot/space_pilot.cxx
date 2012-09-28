#include "space_pilot_inc.h"
#include "space_pilot.h"
#include <vector>

[module(name="SpacePilotLow")];

// a callback list element
typedef struct {
	int event_mask;
	space_pilot_callback cb;
	void* user_data;

} space_pilot_callbacks_entry; 



// a device connection list element
typedef struct{
	// the sensor and keyboard interface. These are both
	// ATL classes which are specified in tdxinput.dll
	// (which is part of 3DxWare)
	CComPtr<ISensor> sensor;
	CComPtr<IKeyboard> keyboard;

	CComPtr<ISimpleDevice> device;

	// a list of callbacks
	std::vector<space_pilot_callbacks_entry*> callbacks;
	// enabled events
	int enabled_events;
} space_pilot_con_entry;


// internal data the commands work with
typedef struct
{
	// the old info to be backwards compatible
	space_pilot_info info;

	// a bitwise list of queued keyups. They are stored in another variable
	// to remember the keydowns between two pollings. Otherwise the 
	// program could "miss" a keypress if it polls too slow.
	int queued_keyups;

	// an abstract interface for the driver that is provided
	// by the COM api
	CComPtr<IUnknown> space_driver;

	std::vector<space_pilot_con_entry*> connections;
	space_pilot_con_entry* active_connection;
	
	// last error
	int last_error;

	// is it our job to close the COM service?
	int close_com;

	// mutex handles for multithreading
	HANDLE	poll_mutex,
			cb_mutex;
} space_pilot_data;

space_pilot_data space_pilot;

// Initialization status of the COM api (used in CoInitializeEx). The setting
// COINIT_APARTMENTTHREADED will cause any com object to trigger events only
// in the message loop. Using space_pilot_set_no_message_loop changes this
// value to COINIT_MULTITHREADED.
DWORD COM_model = COINIT_APARTMENTTHREADED;


// Callbacks for all presented events 
[ event_receiver(com) ]
class COM_callbacks {
	public:
		HRESULT COM_space_pilot_sensor(void);
		HRESULT COM_space_pilot_keyup(int keycode);
		HRESULT COM_space_pilot_keydown(int keycode);
		HRESULT COM_space_pilot_device_change(long reserved );
		void COM_attach(space_pilot_con_entry* con);
};

COM_callbacks COM_interface;
int space_pilot_init_state = 0;


int space_pilot_init()
{
	HRESULT r;

	if (space_pilot_init_state != 0)
		return 1;

	// Initialize basic values
	space_pilot.last_error = SPERROR_NONE;
	space_pilot.active_connection = NULL;
	space_pilot.queued_keyups = 0;

	// ground initialization of the space pilot states
	space_pilot.info.pos[0] = space_pilot.info.pos[1] = space_pilot.info.pos[2] = 0;
	space_pilot.info.rot[0] = space_pilot.info.rot[1] = space_pilot.info.rot[2] = space_pilot.info.rot[3] = 0;
	space_pilot.info.period = 0;
	space_pilot.info.keys = 0;

	// try to initialize the COM api
	r = CoInitializeEx(NULL, COM_model);

	if (r == RPC_E_CHANGED_MODE) {
		space_pilot.last_error = SPERROR_NOTSETUP;
		return 0;
	}

	// if the result is FALSE then the initialization was not
	// successful, because someone in this thread already initalized
	// the COM api. This is generally okay - we just have to remember,
	// that we are not responsible for closing the device
	if (r == S_FALSE)
		space_pilot.close_com = 0;
	else
		space_pilot.close_com = 1;


	// try to get access to the space pilot driver
	r = space_pilot.space_driver.CoCreateInstance(__uuidof(Device));

	if (!SUCCEEDED(r)) {
		space_pilot.last_error = SPERROR_NOTSETUP;
		return 0;
	}

	// create anonymous mutexes (that we don't own initially)
	space_pilot.poll_mutex = CreateMutex(NULL, FALSE, NULL);
	space_pilot.cb_mutex = CreateMutex(NULL, FALSE, NULL);



	// all done. Basic initialization was successful
	space_pilot_init_state = 1;

	return 1;
}



int space_pilot_open(const char* config_name)
{
	space_pilot_con_entry *new_connection;


	space_pilot.last_error = SPERROR_NONE;

	/*
	if (space_pilot.init_state ) {
		space_pilot_init();
	} */

	// check if someone can give us an instance of ISimpleDevice
	new_connection = new space_pilot_con_entry;
	new_connection->enabled_events = SPE_ALL;

	HRESULT r = space_pilot.space_driver.QueryInterface(&new_connection->device);


	if (!SUCCEEDED(r)) {
		delete new_connection;
		space_pilot.last_error = SPERROR_NODEVICE;
		return -1;
	}

	// we got a device. get the sensor and keyboard
	new_connection->device->get_Sensor(&new_connection->sensor);
	new_connection->device->get_Keyboard(&new_connection->keyboard);

	// set the device name (convert it to a basic string first)
	WCHAR buffer[512];
	BSTR bspref;
	// convert the char* to a short* and then the short* to a basic string
	ZeroMemory(buffer, sizeof(buffer));
	MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, 
						config_name, 
						(int)strlen(config_name), 
						buffer, 
						sizeof(buffer)-1);
	bspref = SysAllocString(buffer);
	new_connection->device->LoadPreferences(bspref);
	SysFreeString(bspref); 

	// connect the callback functions
	// _ISimpleDeviceEvents::DeviceChange+=COM_callbacks::COM_space_pilot_device_change;
	COM_interface.COM_attach(new_connection);

	// all properly done
	space_pilot_init_state = 2;


	// append the new connection to the list of connections
	// and return the handle for this connection
	space_pilot.connections.push_back(new_connection);

	return (int)space_pilot.connections.size()-1;
}



void space_pilot_close(int handle)
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return;

	space_pilot_con_entry *con = space_pilot.connections[handle];

	con->device->Disconnect();

	// it might be that the connection to be removed is the actual one
	// if so then nullify the actual connection	
	if (con == space_pilot.active_connection)
		space_pilot.active_connection = NULL;


	// delete all callback information
	std::vector<space_pilot_callbacks_entry*>::iterator iter;
	for (iter = con->callbacks.begin(); iter != con->callbacks.end(); iter++)
		delete (*iter);

	// remove the connection from the list
	space_pilot.connections.erase(space_pilot.connections.begin()+handle);

	// remove the connection itself
	delete con;


	// if there are no more connections then we can also close 
	// the COM stuff (in case we opened it)
	if (space_pilot.connections.size()==0) {
		space_pilot_init_state = 0;

		if (space_pilot.close_com)
			CoUninitialize();

		// also destroy the mutexes
		CloseHandle(space_pilot.poll_mutex);
		CloseHandle(space_pilot.cb_mutex);
	}
}



void space_pilot_set_no_message_loop()
{
	COM_model = COINIT_MULTITHREADED;
}



void space_pilot_check_state()
{
	// Does nothing on WIN32
}



void space_pilot_grab_focus(int handle)
{
	if (handle<0 ||handle>=(int)space_pilot.connections.size())
		return;

	// release the old connection
	if (space_pilot.active_connection)
		space_pilot.active_connection->device->Disconnect();

	// open the new connection
	space_pilot.active_connection = space_pilot.connections[handle];
	space_pilot.active_connection->device->Connect();
}



void space_pilot_release_focus()
{
	if (!space_pilot.active_connection)
		return;

	space_pilot.active_connection->device->Disconnect();
	space_pilot.active_connection = NULL;
}



void space_pilot_poll_motion(double* pos, double* rot)
{
	if (!space_pilot.active_connection)
		return;

	space_pilot_poll(NULL);

	// copy the values
	WaitForSingleObject(space_pilot.poll_mutex, INFINITE );
	memcpy(rot, &space_pilot.info.rot, sizeof(space_pilot.info.rot));
	memcpy(pos, &space_pilot.info.pos, sizeof(space_pilot.info.pos));
	ReleaseMutex(space_pilot.poll_mutex);
}



unsigned int space_pilot_poll_keys()
{
	if (!space_pilot.active_connection)
		return 0;

	space_pilot_poll(NULL);
	// FIXME: can this cause an inconstistent state?
	return space_pilot.info.keys;
}



double space_pilot_poll_period()
{
	if (!space_pilot.active_connection)
		return 0;

	space_pilot_poll(NULL);
	// FIXME: can this cause an inconsistent state?
	return space_pilot.info.period;
}



void space_pilot_poll(space_pilot_info* state_dest)
{
	// no need to read the values explicitly as this is done in the hook
	// method COM_space_pilot_sensor and friends. The only thing that has
	// to be done here is to make sure that the user doesn't miss a 
	// keypress. For this, the pressed and released keys were saved in 
	// different variables. The user now gets only the pressed keys, then
	// the pressed key info gets merged with the released key info and the
	// latter variable gets resetted.

	WaitForSingleObject(space_pilot.poll_mutex, INFINITE );
	// copy the values if neccessary
	if (state_dest)
		memcpy(state_dest, &space_pilot.info, sizeof(space_pilot_info));

	// merge keyups and keydowns and reset keyups
	space_pilot.info.keys &= ~space_pilot.queued_keyups;
	space_pilot.queued_keyups = 0;

	ReleaseMutex(space_pilot.poll_mutex);
}



void space_pilot_add_callback(int handle, int event_mask, space_pilot_callback callback, void* user_data)
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return;

	space_pilot_callbacks_entry *new_callback = new space_pilot_callbacks_entry;

	new_callback->event_mask = event_mask;
	new_callback->user_data = user_data;
	new_callback->cb = callback;

	space_pilot.connections[handle]->callbacks.push_back(new_callback);
}



void space_pilot_remove_callback(int handle, space_pilot_callback callback, void* user_data)
{
	if (handle<0 ||handle>=(int)space_pilot.connections.size())
		return;

	std::vector<space_pilot_callbacks_entry*> cbs = space_pilot.connections[handle]->callbacks;

	// delete all entries that match the callback/user data
	std::vector<space_pilot_callbacks_entry*>::iterator iter;
	iter = cbs.begin();

	while(iter != cbs.end()) {
		if ((*iter)->cb == callback && (*iter)->user_data == user_data) 
			cbs.erase(iter);

		iter++;
	}
}



int space_pilot_get_error()
{
	return space_pilot.last_error;
}



void space_pilot_disable_events(int handle, int event_mask)
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return;

	space_pilot.connections[handle]->enabled_events &= ~event_mask;
}



unsigned int space_pilot_get_enabled_events(int handle) 
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return 0;

	return space_pilot.connections[handle]->enabled_events;
}




/****************************************************************
 Win32 specific helpers
 ****************************************************************/

void _space_pilot_start_callbacks(int events) 
{
	if (!space_pilot.active_connection || 
		!space_pilot.active_connection->callbacks.size())
		return;

	// filter events according to the information that was provided by
	// the method space_pilot_disable_events()
	int mask = events & space_pilot.active_connection->enabled_events;

	// create a copy of the space pilot info stuff.
	// only in case someone changes something in the callback
	space_pilot_info info_copy;
	memcpy(&info_copy, &space_pilot.info, sizeof(space_pilot_info));
	// apply the queued keyups information
	info_copy.keys &= ~space_pilot.queued_keyups;

	// for iteration over the callback entries we make a copy of the vector
	// because it might be possible for a callback function to change
	// the active connection - then our iterator would become invalid
	std::vector<space_pilot_callbacks_entry*> active_copy = space_pilot.active_connection->callbacks;

	std::vector<space_pilot_callbacks_entry*>::iterator iter;
	for (iter = active_copy.begin();
		 iter != active_copy.end();
		 iter++) {
			 if ((*iter)->event_mask & mask)
				 (*iter)->cb(events & (*iter)->event_mask, &info_copy, (*iter)->user_data);
	}
}



HRESULT COM_callbacks::COM_space_pilot_sensor(void)
{
	if (!space_pilot.active_connection)
		return S_OK;

	WaitForSingleObject(space_pilot.cb_mutex, INFINITE );
	WaitForSingleObject(space_pilot.poll_mutex, INFINITE );


	double tx, ty, tz, rx, ry, rz, angle;
	// a sensor input was received. Check whether something new has
	// happened to translation or rotation and trigger the
	// appropriate events
	CComPtr<IAngleAxis> rotation;
	space_pilot.active_connection->sensor->get_Rotation(&rotation);
	rotation->get_X(&rx);
	rotation->get_Y(&ry);
	rotation->get_Z(&rz);
	rotation->get_Angle(&angle);
	rotation.Release();

	// get the translation
	CComPtr<IVector3D> translation;
	space_pilot.active_connection->sensor->get_Translation(&translation);
	translation->get_X(&tx);
	translation->get_Y(&ty);
	translation->get_Z(&tz);
	translation.Release();


	int mask = 0;
	// according to the old information try to find out which event happened
	if (space_pilot.info.pos[0] != tx || 
	    space_pilot.info.pos[1] != ty || 
	    space_pilot.info.pos[2] != tz)
		mask = mask | SPE_TRANSLATION;

	if (space_pilot.info.rot[0] != rx || 
	    space_pilot.info.rot[1] != ry || 
	    space_pilot.info.rot[2] != rz ||
	    space_pilot.info.rot[3] != angle)
		mask = mask | SPE_ROTATION;


	// update the rotation
	space_pilot.info.rot[0] = rx;
	space_pilot.info.rot[1] = ry;
	space_pilot.info.rot[2] = rz;
	space_pilot.info.rot[3] = angle;

	// update the translation
	space_pilot.info.pos[0] = tx;
	space_pilot.info.pos[1] = ty;
	space_pilot.info.pos[2] = tz;

	// update the period
	space_pilot.active_connection->sensor->get_Period(&space_pilot.info.period);


	ReleaseMutex(space_pilot.poll_mutex);

	// trigger callbacks 
	_space_pilot_start_callbacks(mask);
	ReleaseMutex(space_pilot.cb_mutex);

	return S_OK;
}



HRESULT COM_callbacks::COM_space_pilot_keyup(int keycode)
{
	if (!space_pilot.active_connection || keycode<1 ||keycode>32)
		return S_OK;

	WaitForSingleObject(space_pilot.cb_mutex, INFINITE );
	WaitForSingleObject(space_pilot.poll_mutex, INFINITE );


	// update key info
	space_pilot.queued_keyups |= (1<<(keycode-1));

	ReleaseMutex(space_pilot.poll_mutex);

	// trigger callbacks
	_space_pilot_start_callbacks(SPE_KEYUP);

	ReleaseMutex(space_pilot.cb_mutex);

	return S_OK;
}



HRESULT COM_callbacks::COM_space_pilot_keydown(int keycode)
{
	if (!space_pilot.active_connection || keycode<1 ||keycode>32)
		return S_OK;

	WaitForSingleObject(space_pilot.cb_mutex, INFINITE );
	WaitForSingleObject(space_pilot.poll_mutex, INFINITE );

	// update key info
	space_pilot.info.keys |= (1<<(keycode-1));

	// also mark this key as not being up anymore
	space_pilot.queued_keyups &= ~(1<<(keycode-1));

	ReleaseMutex(space_pilot.poll_mutex);

	// trigger callbacks
	_space_pilot_start_callbacks(SPE_KEYDOWN);

	ReleaseMutex(space_pilot.cb_mutex);

	return S_OK;
}



HRESULT COM_callbacks::COM_space_pilot_device_change(long reserved)
{
	// trigger callbacks
	_space_pilot_start_callbacks(SPE_DEVICECHANGE);
	return S_OK;
}


void COM_callbacks::COM_attach(space_pilot_con_entry* con)
{
    __hook(&_ISimpleDeviceEvents::DeviceChange, con->device, &COM_callbacks::COM_space_pilot_device_change);
	__hook(&_ISensorEvents::SensorInput, con->sensor, &COM_callbacks::COM_space_pilot_sensor);
	__hook(&_IKeyboardEvents::KeyDown, con->keyboard, &COM_callbacks::COM_space_pilot_keydown);
	__hook(&_IKeyboardEvents::KeyUp, con->keyboard, &COM_callbacks::COM_space_pilot_keyup);
}
