/**
\page space_pilot space_pilot header

\section intro What is this about?
	This package contains a C like interface for for communicating with 3dconnexion devices that are handled by 3DxWare.
	these are multi-axis and multi-button devices to retrieve relative or absolute motion information in a 3D space. 
	See the official documentation on 3dconnexion.com for a more matured explanation on this type of controlling device.

	You probably don't want to use this package in your C++ projects; there is a more comfortable C++ interface space_input.
	However, if you are programming in another language than C++ then this library can provide a simple way to access 
	3DxWare driven devices.

	Please refer to the documentation of space_input for more description, examples and trouble shooting.

\section usage Examples
	The use of the methods is quite straight-forward. To get information you can either use polling or callbacks. Here is a
	small example that explains how to do things:
	\code
	#include <stdio.h>
	#include "spacepilot.h"

	static void sensor_callback(int mask, space_pilot_info *info, void* user_data) 
	{
		printf("Translation is: %f, %f, %f\n", 
			info->pos[0], info->pos[1], info->pos[2]);

		// Also possible:
		double pos[3], rot[4];
		space_pilot_poll_motion(pos, rot);
		printf("Rotation axis is: %f, %f, %f, Angle: %f\n", 
			rot[0], rot[1], rot[2], rot[3]);

	}


	void main() 
	{
		// we are not having a message loop
		space_pilot_set_no_message_loop();

		// Initialize the device
		if (!space_pilot_init()) {
			printf("Init failed with code: %d\n", space_pilot_get_error());
			return;
		}

		// Try to open the device and get a handle
		int handle = space_pilot_open("MyConfiguration");

		// If the handle is -1 the opening failed
		if (handle == -1) {
			printf("Open failed with code: %d\n", space_pilot_get_error());
			return;
		}

		// Grab focus for this connection
		space_pilot_grab_focus(handle);

		// Add the callback
		space_pilot_add_callback(handle, SPE_ROTATION | SPE_TRANSLATION, 
					 sensor_callback, NULL);

		while(1) {};
	}
	\endcode

	With every call to "open" a new handle is created (which represents a new connection). You can then either use
	polling and create your own polling loop or you can attach as many callbacks as you want which get triggered 
	automatically on every update. 
*/

#pragma once

/*!
  \file space_pilot.h

 A low level api for 3d-devices by 3dconnexion.
 This library provides a low level api for devices by 3dconnexion (eg. Space Pilot, Space Navigator).
 (TODO: More explanation, examples ...)

  \author Joachim Staib
  \date   2007-12-10
*/


/**
 \defgroup Enums Structs, Enums and Typedefs
*/
/*@{*/
/**
 * Information on various states of the device.
 * This structure is given to a callback function to read requested information.
*/
typedef struct {
	double rot[4];		/**< The rotation values of the device (x, y, z, angle) */
	double pos[3];		/**< The position values of the device (x, y, z) */
	double period;		/**< The period of the device (the elapsed time since last poll) */
	unsigned int keys;	/**< A bitmask that represents the state of the keys (if any) */
} space_pilot_info;


/**
 * Designators to attach a callback.
 * When adding a callback you can define a bitmask for which events of the device the callback
 * shall be triggered. For example "SPE_ROTATION | SPE_TRANSLATION" would trigger the callback
 * whenever there is a change in the position OR rotation
*/
enum space_pilot_event {
	SPE_NONE	= 1<<0,		/**< On no event */
	SPE_ROTATION	= 1<<1,		/**< On rotation changes */
	SPE_TRANSLATION	= 1<<2,		/**< On position changes */
	SPE_KEYDOWN	= 1<<3,		/**< On button press */
	SPE_KEYUP	= 1<<4,		/**< On button release */
	SPE_DEVICECHANGE= 1<<5,		/**< On device change (plugged, unplugged ...) */
	SPE_ALL		= 0xFF		/**< On all events */
};


/**
 * Designators for device errors.
 * When calling space_pilot_init or space_pilot_open errors can occor. The following list contains
 * a list of possible errors.
*/
enum space_pilot_error {
	SPERROR_NONE,		/**< Everything went well */
	SPERROR_NODEVICE,	/**< No device could be found */
	SPERROR_NOCONNECTION,	/**< Device present, but connection not possible */
	SPERROR_NOTREADY,	/**< The device is not ready */
	SPERROR_NOTSETUP	/**< The device might be present, but the OS cannot establish communication ways */
};



/**
 * The callback function.
 * To connect a callback to the device you should define a callback function that looks like this.
 * @param event a bitmask of the event, that occured
 * @param info a pointer to a space_pilot_info struct that contains updated values. Contents must not be changed.
 * @param user_data special user data the user defined when adding the callback
 * 
 * @see space_pilot_add_callback
*/
typedef void(*space_pilot_callback) (int event, space_pilot_info* info, void* user_data);
/*@}*/



/**
 \defgroup Funcs Methods for controlling the device
*/
/*@{*/
/** 
 Initialize the device.
 To initialize the device call this method. It should be called as soon as possible in your program.
 Returns 0 if the initialization failed or 1 on success. The exact error can then be read by calling
 space_pilot_get_error().
 @returns 0 on error, 1 on success

 @see space_pilot_get_error()
*/
int space_pilot_init();



/**
 Opens the device.
 To open the space pilot call this method. It sends the preferred configuration for your application,
 connects to the device, starts the message thread if necccessary and initializes callbacks.
 Returns 0 if the initialization failed or 1 on success. The exact error can then be read by calling
 space_pilot_get_error().
 @returns -1 if failed, otherwise new handle

 @see space_pilot_get_error()
*/
int space_pilot_open(const char* config_name);



/** 
 Closes the device.
 If you don't need the service of the device any longer you can close it using this method. It disconnects
 the device, stops the message thread and frees all callback information. */
void space_pilot_close(int handle);



/**
 Set whether the program using this device has an own message loop.
 Call this once and _BEFORE_ space_pilot_init() if your application doesn't use a message loop
 (eg. console applications). This will take all neccessary steps to ensure the callbacking system.
 If you don't use callbacks you don't need to call this method. 
 Internally this will detach device updates from the message loop and thus can call callbacks at any
 time, which might cause concurrency problems. Be aware of using multi-threading techniques and avoid
 drawing in the callback function if you use this method. */
void space_pilot_set_no_message_loop();



/**
 Grabs focus of the device. Use this method whenever your window gets focussed
 @param handle The handle to grab focus for
*/
void space_pilot_grab_focus(int handle);



/**
 Releases the focus of the device.
*/
void space_pilot_release_focus();



/**
 Polls the device for motion.
 If you are not using callbacks you can use this method to get the actual motion information.
 @param pos a double[3] that shall contain the position information
 @param rot a double[4] that shall contain the rotation information

 @see space_pilot_poll_keys
 @see space_pilot_poll_period
*/
void space_pilot_poll_motion(double* pos, double* rot);

/** 
 Polls the device for the keys bitmap.
 If you are not using callbacks you can use this method to get the actual keys bitmap.
 @returns the actual keys bitmap

 @see space_pilot_poll_motion
 @see space_pilot_poll_period
 @see space_pilot_poll
*/
unsigned int space_pilot_poll_keys();

/** 
 Polls the device for the period.
 If you are not using callbacks you can use this method to get the actual period which is
 the time since the device was updated.
 @returns the actual period in seconds

 @see space_pilot_poll_motion
 @see space_pilot_poll_keys
 @see space_pilot_poll
*/
double space_pilot_poll_period();

/**
 Polls every info that the device can return.
 If you are not using callbacks you can use thisd method to get the space pilot info
 @param state_dest a pointer of a space_pilot_info struc to copy the values to

 @see space_pilot_poll_motion
 @see space_pilot_poll_keys
 @see space_pilot_poll_period
*/
void space_pilot_poll(space_pilot_info* state_dest);



/**
 Adds a callback to report device changes.
 Using this method you can add a callback that shall be triggered whenever a certain state
 of the device changes.
 @param handle a handle to add the callback for
 @param event_mask a combination of space_pilot_event elements that define an OR'ed mask
 @param callback the callback function
 @param user_data user defined data

 @see space_pilot_poll_motion
 @see space_pilot_poll_keys
 @see space_pilot_poll_period
*/
void space_pilot_add_callback(int handle, int event_mask, space_pilot_callback callback, void* user_data);


/**
 Removes a callback.
 You can remove a callback if it is no longer needed.
 @param handle a handle to remove the callback from
 @param callback the callback function to remove
 @param user_data the user data the callback was added with
*/
void space_pilot_remove_callback(int handle, space_pilot_callback callback, void* user_data);


/**
 Returns the error of the last operation.
 The methods space_pilot_open and space_pilot_init can generate errors, which is one of space_pilot_error.
 The flags are cleared automatically before calling a method that changes the error code.
 @returns the error code (one of space_pilot_error)
*/
int space_pilot_get_error();



/**
 Enable/disable certain events.
 You can disable certain events of the device if they are not needed any longer. When the device reports a
 change to a disabled event no callbacks that are triggered on this events are called. To enable all 
 events again you can use space_pilot_disable_events(SPE_NONE);
 @param handle a handle to disable events for
 @param event_mask a bitmask of space_pilot_event that defines events to disable
*/
void space_pilot_disable_events(int handle, int event_mask);

/**
 Get enabled events.
 Returns a bitmask of the events that can cause callbacks. Set this value with space_pilot_disable_events
 @param handle the handle to get information for
 @returns bitmask of events (out of space_pilot_event) */
unsigned int space_pilot_get_enabled_events(int handle);
/*@}*/

