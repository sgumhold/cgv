
/*****************************************************************************
 *       X11-Specific implementation                                         *
 *       (for WIN32-Implementation see below)                                *
 *****************************************************************************/

#include "space_pilot.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <vector>


#define XHigh32( Value )        (((Value)>>16)&0x0000FFFF)
#define XLow32( Value )         ((Value)&0x0000FFFF)

#define SPACE_BUTTON_START 0x43
#define SPACE_BUTTON_FIT 0x5F


// a callback list element
typedef struct {
	int event_mask;
	space_pilot_callback cb;
	void* user_data;

} space_pilot_callbacks_entry;



// a space pilot information window instance
typedef struct {
	Window input_window;
	// list of callbacks
	std::vector<space_pilot_callbacks_entry*> callbacks;
	// enabled events
	int enabled_events;


} space_pilot_con_entry;



// some messages the command window can understand. There are probably much more
// but they are not DOCUMENTED!
enum command_messages { 
	CM_NOCOMMAND, 
	CM_APPWINDOW = 27695,
	CM_SENSITIVITY
};


// data the commands work with
typedef struct {
	// the old info to be backwards compatible
	space_pilot_info info;

	// X11 Atoms
	// They define events and get referred by the xserver when
	// a corresponding event occurs
	Atom 	motion_event,
		button_press_event,
		button_release_event,
		command_event;

	// The window to which the space pilot is connected
	std::vector<space_pilot_con_entry*> connections;
	space_pilot_con_entry *active_connection;
	Display *display;

	// last error
	int last_error;

	// released keys
	unsigned int released_keys;

	// Thread info
 	pthread_t thread_id;
 	pthread_mutex_t poll_lock;
	pthread_mutex_t callback_lock;
 	pthread_mutex_t check_lock;

} space_pilot_data;


// the actual space pilot
space_pilot_data space_pilot;

int space_pilot_init_state = 0;

// declaration of the polling thread
void* x11_spacepilot_thread_loop(void* arg);
// private helper function declarations
// (see below for definitions)
int _realize_spacepilot_motion_event(XEvent* event);
int _realize_spacepilot_button_event(XEvent* event);
void _space_pilot_update();
Bool _pred_is_spacepilot_event(Display* display, XEvent* event, XPointer data);
int x11_send_device_command(short* data);
void _set_active_connection(space_pilot_con_entry *new_active);





// initialize the space pilot
int space_pilot_init() 
{
	space_pilot.last_error = SPERROR_NONE;

	if (space_pilot_init_state != 0)
		return 1;

	// ground initialization of the space pilot states
	space_pilot.info.pos[0] = space_pilot.info.pos[1] = space_pilot.info.pos[2] = 0;
	space_pilot.info.rot[0] = space_pilot.info.rot[1] = space_pilot.info.rot[2] = space_pilot.info.rot[3] = 0;
	space_pilot.info.period = 0;
	space_pilot.info.keys = 0;
	space_pilot.released_keys = 0;
	

	// open the root display
	space_pilot.display = XOpenDisplay(NULL);

	// check the Xserver for the existence of needed basic events and get their handles
	space_pilot.motion_event  = XInternAtom(space_pilot.display, "MotionEvent", True);
	space_pilot.command_event = XInternAtom(space_pilot.display, "CommandEvent", True );

	space_pilot.button_press_event = XInternAtom(space_pilot.display, "ButtonPressEvent", True );
	space_pilot.button_release_event = XInternAtom(space_pilot.display, "ButtonReleaseEvent", True);

	// check if all events are existing and have handles
	// otherwise the X-Server might have a problem
	if (!space_pilot.motion_event ||
	    !space_pilot.button_press_event ||
	    !space_pilot.button_release_event ||
	    !space_pilot.command_event) {
		space_pilot.last_error = SPERROR_NODEVICE;
		return 0;
	}

	space_pilot_init_state = 1;

	return 1;

}



// open the space pilot
int space_pilot_open(char* config_name) 
{
 	Window root;

	space_pilot_con_entry *newcon;
	int list_length;



	space_pilot.last_error = SPERROR_NONE;

	if (space_pilot_init_state < 1) {
		space_pilot.last_error = SPERROR_NOTSETUP;
		return -1;
	}

	newcon = new space_pilot_con_entry;
	newcon->enabled_events = SPE_ALL;


	root = DefaultRootWindow(space_pilot.display);

	/* The space pilot events can only be polled when a client window is associated
	   to the controlling window for the device. 
	   Here, an InputOnly-Window is defined and space-pilot events are connected
	   to it. In the message loop the states for this window can then be read */

	// Create the window
 	newcon->input_window = XCreateWindow(space_pilot.display,
					    root, 
					     0, 0, 		// Position
					     1, 1, 		// Dimensions
					     0,			// Border width
					     0,			// Depth
					     InputOnly,		// Class
					     NULL,		// Visual
					     0,			// Values mask
					     NULL);		// Value attributes list

	// FIXME: check whether the window was actually created

	// enable input events and give a fancy name
	XWMHints *wmhints    = XAllocWMHints();
	XClassHint *classhints = XAllocClassHint();

	if (!wmhints || !classhints) {
		XDestroyWindow(space_pilot.display, newcon->input_window);
		delete newcon;
		space_pilot.last_error = SPERROR_NOTSETUP;
		return -1;
	}

	wmhints->initial_state = NormalState;
	wmhints->input = True;
	wmhints->flags = StateHint | InputHint;

	classhints->res_name = config_name;

	XSetWMProperties(space_pilot.display, newcon->input_window, NULL, NULL, NULL,
		   0, NULL, wmhints, classhints );


	// append the new input window to the list of input windows
	space_pilot.connections.push_back(newcon);


	if (space_pilot_init_state != 2) {
		space_pilot_init_state=2;

		// if this is the first time the device is opened then set
		// the currently active device to this
		if (!space_pilot.active_connection)
			space_pilot.active_connection = newcon;


	 	// processing is done in a separate thread.
 		// this is neccessary because X11 doesn't support event handlers so we need 
 		// to establish an own message queue which captures ClientMessage-Events
 		pthread_mutex_init(&space_pilot.poll_lock, NULL);
		pthread_mutex_init(&space_pilot.callback_lock, NULL);
		pthread_mutex_init(&space_pilot.check_lock, NULL);

		pthread_create(&space_pilot.thread_id, NULL, x11_spacepilot_thread_loop, NULL);
	}

	return space_pilot.connections.size()-1;	

}




// close the space pilot
void space_pilot_close(int handle) 
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return;

	space_pilot_con_entry *con = space_pilot.connections[handle];

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

	// delete the window for that connection
	XDestroyWindow(space_pilot.display, con->input_window);

	// remove the connection itself
	delete con;
}



void space_pilot_set_no_message_loop() 
{
	// Does nothing on linux :(
}



void space_pilot_grab_focus(int handle) 
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return;

	space_pilot_con_entry *con = space_pilot.connections[handle];

	_set_active_connection(con);
}


void space_pilot_release_focus()
{
	if (!space_pilot.active_connection)
		return;

	space_pilot.active_connection = NULL;
}





// poll motion information
void space_pilot_poll_motion(double* pos, double* rot) 
{
	space_pilot.last_error = SPERROR_NONE;

	// lock the values
 	pthread_mutex_lock(&space_pilot.poll_lock);

	// copy values
	memcpy(pos, space_pilot.info.pos, sizeof(double)*3);
	memcpy(rot, space_pilot.info.rot, sizeof(double)*4);

 	pthread_mutex_unlock(&space_pilot.poll_lock);
}



// poll key information
unsigned int space_pilot_poll_keys() 
{
	space_pilot.last_error = SPERROR_NONE;

	// lock the values 
 	pthread_mutex_lock(&space_pilot.poll_lock);

	int keys = space_pilot.info.keys;

	// apply the released buttons state and clear the flag
	space_pilot.info.keys = (space_pilot.info.keys | space_pilot.released_keys) ^ space_pilot.released_keys;
	space_pilot.released_keys = 0;


 	pthread_mutex_unlock(&space_pilot.poll_lock);

	return keys;
}



// poll period information
double space_pilot_poll_period() 
{
	space_pilot.last_error = SPERROR_NONE;

	// lock values
 	pthread_mutex_lock(&space_pilot.poll_lock);

	double period = space_pilot.info.period;

	// unlock values
 	pthread_mutex_unlock(&space_pilot.poll_lock);

	return period;
}



// poll every information
void space_pilot_poll(space_pilot_info* state_dest) 
{
	space_pilot.last_error = SPERROR_NONE;

	// lock values
 	pthread_mutex_lock(&space_pilot.poll_lock);

	memcpy(state_dest, &space_pilot.info, sizeof(space_pilot_info));

	// apply the released buttons state and clear the flag
	space_pilot.info.keys = (space_pilot.info.keys | space_pilot.released_keys) ^ space_pilot.released_keys;
	space_pilot.released_keys = 0;

	// unlock values
 	pthread_mutex_unlock(&space_pilot.poll_lock);
}



// add a callback to the queue
void space_pilot_add_callback(int handle, int event_mask, space_pilot_callback callback, void* user_data) 
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return;

	space_pilot_con_entry *con = space_pilot.connections[handle];

	space_pilot_callbacks_entry* new_callback;


	// create a new callback structure
	new_callback = new space_pilot_callbacks_entry;
	new_callback->event_mask = event_mask;
	new_callback->cb = callback;
	new_callback->user_data = user_data;

	con->callbacks.push_back(new_callback);
}



// remove a callback from the queue
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



// get the last error
int space_pilot_get_error() {
	return space_pilot.last_error;
}



// set disabled events
void space_pilot_disable_events(int handle, int event_mask)
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return;

	space_pilot.connections[handle]->enabled_events = (space_pilot.connections[handle]->enabled_events | event_mask) ^ event_mask;
}


unsigned int space_pilot_get_enabled_events(int handle) 
{
	if (handle<0 || handle>=(int)space_pilot.connections.size())
		return 0;

	return space_pilot.connections[handle]->enabled_events;
}





/* Private helper methods */
/**************************/


// evaluate a motion event
int _realize_spacepilot_motion_event(XEvent* event)
{
	int mask = 0;

	// check if we are on the right event
	if (event->type != ClientMessage || event->xclient.message_type != space_pilot.motion_event)
		return 0;

	pthread_mutex_lock(&space_pilot.poll_lock);

	// save the old info
	space_pilot_info old_info;
	memcpy(&old_info, &space_pilot.info, sizeof(space_pilot_info));

	// get the translation
	space_pilot.info.pos[0] = event->xclient.data.s[2];
	space_pilot.info.pos[1] = event->xclient.data.s[3];
	space_pilot.info.pos[2] = -event->xclient.data.s[4];

	// get the rotation (in unix we only get three angles)
	space_pilot.info.rot[0] = event->xclient.data.s[5];
	space_pilot.info.rot[1] = event->xclient.data.s[6];
	space_pilot.info.rot[2] = event->xclient.data.s[7];
	double len = sqrt(space_pilot.info.rot[0]*space_pilot.info.rot[0] + 
			  space_pilot.info.rot[1]*space_pilot.info.rot[1] +
			  space_pilot.info.rot[2]*space_pilot.info.rot[2]);
	space_pilot.info.rot[3] = len;
	if (len != 0) {
		space_pilot.info.rot[0] /= len;
		space_pilot.info.rot[1] /= len;
		space_pilot.info.rot[2] /= -len;
	}

	// get the period
	space_pilot.info.period = event->xclient.data.s[8];


	// according to the old information try to find out which event happened
	if (space_pilot.info.pos[0] != old_info.pos[0] || 
	    space_pilot.info.pos[1] != old_info.pos[1] || 
	    space_pilot.info.pos[2] != old_info.pos[2])
		mask = mask | SPE_TRANSLATION;

	if (space_pilot.info.rot[0] != old_info.rot[0] || 
	    space_pilot.info.rot[1] != old_info.rot[1] || 
	    space_pilot.info.rot[2] != old_info.rot[2] ||
	    space_pilot.info.rot[3] != old_info.rot[3])
		mask = mask | SPE_ROTATION;

	pthread_mutex_unlock(&space_pilot.poll_lock);

	return mask;
}




int _realize_spacepilot_button_event(XEvent* event)
{
	// check if we are on the right event
	if (event->type != ClientMessage || 
	    (event->xclient.message_type != space_pilot.button_press_event &&
	     event->xclient.message_type != space_pilot.button_release_event))
		return 0;

	pthread_mutex_lock(&space_pilot.poll_lock);

	short but = event->xclient.data.s[2]-1;

	
	// if the button is pressed then set the button flags
	if (event->xclient.message_type == space_pilot.button_press_event) {
		space_pilot.info.keys = space_pilot.info.keys | (1<<but);
		space_pilot.released_keys = (space_pilot.released_keys | (1<<but)) ^ (1<<but);

	} else {
		space_pilot.released_keys |= (1<<but);
		pthread_mutex_unlock(&space_pilot.poll_lock);
		return SPE_KEYUP;
	}

	pthread_mutex_unlock(&space_pilot.poll_lock);

	return SPE_KEYDOWN;
}




Bool _pred_is_spacepilot_event(Display* display, XEvent* event, XPointer data)
{
	if (event->type != ClientMessage)
		return False;

	if (event->xclient.message_type == space_pilot.motion_event ||
	    event->xclient.message_type == space_pilot.button_press_event ||
	    event->xclient.message_type == space_pilot.button_release_event)
		return True;

	return False;
}




int x11_send_device_command(short* data) 
{
 	Window root;
	Atom prop_identifier;
	int prop_format, i;
	unsigned long item_count;
	unsigned long bytes_count;
	unsigned char *prop_data;
	XEvent command;
	int connect_result;
	XTextProperty command_window_name;

	root = DefaultRootWindow(space_pilot.display);

	XFlush(space_pilot.display);

	// Get the window that controls the space pilot

 	XGetWindowProperty(space_pilot.display,
			   root, 
			   space_pilot.command_event, 			// property to get
			   0,						// offset
			   1, 						// length in 32bit-multiples
			   False,					// property deleted?
                     	   AnyPropertyType, 				// Atom identifier for property
			   &prop_identifier, 
			   &prop_format, 
			   &item_count,
                     	   &bytes_count, 
			   &prop_data);

	// FIXME: check whether this command executed successfully

	// check if we got any data
	if (!prop_data) {
		space_pilot.last_error = SPERROR_NODEVICE;
		return 0;
	}

	Window command_window = *(Window*) prop_data;

	// check if we got the right data by testing whether our data is a window with a name
	if (!XGetWMName(space_pilot.display, command_window, &command_window_name))
		return 0;

	// Ask the control window to connect the InputOnly window
	// FIXME: will this work on 64-bit systems? I don't think so
	command.type = ClientMessage;
	command.xclient.format = 16;
	command.xclient.send_event = False;
	command.xclient.display = space_pilot.display;
	command.xclient.window = command_window;
	command.xclient.message_type = space_pilot.command_event;

	for (i=0; i<10; i++)
		command.xclient.data.s[i] = data[i];
	
	connect_result = XSendEvent(space_pilot.display, command_window, 0, 0, &command);


	XFree(prop_data);
	XFlush(space_pilot.display);

	return (connect_result != 0);	
}



void _set_active_connection(space_pilot_con_entry *new_active) 
{
	// lock the looping thread
	pthread_mutex_lock(&space_pilot.check_lock);

	// Tell the driver that the window wants to connect to the device
	short command_data[10];
	command_data[0] = (short) XHigh32(((unsigned long)new_active->input_window));
	command_data[1] = (short) XLow32(((unsigned long)new_active->input_window));
       	command_data[2] = CM_APPWINDOW;
	x11_send_device_command(command_data);

	space_pilot.active_connection = new_active;

	pthread_mutex_unlock(&space_pilot.check_lock);
}





void* x11_spacepilot_thread_loop(void* arg)
{
	XEvent event;
	int mask;
	int filtered_mask;

	while(1) {
		// wait for our event and block the process
		while(1) {
			pthread_mutex_lock(&space_pilot.check_lock);
			if (XCheckIfEvent(space_pilot.display, &event, _pred_is_spacepilot_event, NULL)) {
				pthread_mutex_unlock(&space_pilot.check_lock);
				break;
			}

			pthread_mutex_unlock(&space_pilot.check_lock);
			usleep(10000);
		}

		mask = 0;

		// update information
		mask |= _realize_spacepilot_motion_event(&event);
		mask |= _realize_spacepilot_button_event(&event);


		// only do callbacking if there is an active connection
		if (!space_pilot.active_connection)
			continue;

		filtered_mask = mask & space_pilot.active_connection->enabled_events;


		// copy the information for handover
		space_pilot_info info;
		memcpy(&info, &space_pilot.info, sizeof(space_pilot_info));
		info.keys = (info.keys | space_pilot.released_keys) ^ space_pilot.released_keys;

		pthread_mutex_lock(&space_pilot.callback_lock);

		// for iteration over the callback entries we make a copy of the vector
		// because it might be possible for a callback function to change
		// the active connection - then our iterator would become invalid
		std::vector<space_pilot_callbacks_entry*> active_copy = space_pilot.active_connection->callbacks;
	
		std::vector<space_pilot_callbacks_entry*>::iterator iter;
		for (iter = active_copy.begin();
			iter != active_copy.end();
			iter++) {
				if ((*iter)->event_mask & mask)
					(*iter)->cb(mask & (*iter)->event_mask, &info, (*iter)->user_data);
		}

		pthread_mutex_unlock(&space_pilot.callback_lock);

	}
	return NULL;
} 

