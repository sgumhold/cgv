/** 
 \mainpage space_input Description

 \section intro_sec What is this about?
	This package contains a simple class for communicating with 3dconnexion devices that are handled by 3DxWare.
	these are multi-axis and multi-button devices to retrieve relative or absolute motion information in a 3D space. 
	See the official documentation on 3dconnexion.com for a more matured explanation on this type of controlling device.

	To use the class in VisualC++ do the following:
		- Copy the spacepilot.dll to the location of you executable
		- Add the dependency "spacepilot.lib" in your project settings
		- Add the files spacepilot.h, space_input.h and space_input.cpp to your project

	To use the class in Linux (with autotools) do the following:
		- Make sure that libspacepilot.so is in a directory where the linker can find it
		- Tell the linker to add libspacepilot.so (for example by adding -lspacepilot to LD_ADD)
		- Add the files spacepilot.h, space_input.h and space_input.cpp to your project

	The class is part of the namespace cgv::input.

	An alternative to using the class space_input would be to use
		- the C interface this class wraps (then you have to deal with handles)
		- the COM interface in Windows / The XEvent interface on linux. This is the layer the C interface
		  is built on. Using this layer itself is not recommended, as it forces you to bad program
		  design, instability and depending on many external headers. 

	Use this class / the c interface for an easy way to access the information of Space-*** devices 
	One feature is that multiple instances of this class can be handled. If you for example have two 3D spaces in your
	program but only one shall be controllable by the device at a time you can nevertheless create completely isolated
	instances of space_input and call grab_focus() from the instance that shall have control. It is also possible
	to provide a unique configuration name for every instance. See the example "MultiFocusTest" for a demonstration.

	To have the callback timings synchronized with your window message loop, the callbacks can only be triggered while
	the message loop dispatches messages. Because of this serialized behavior you do not need to care about threading
	concurrency problems (Unfortunately this is only true for the Windows Implementation. See the trouble shooting section
	for more details on that). Also this makes it neccessary to have a message loop in your program. If you don't have one
	you can decouple the callbacking from the message loop by calling space_input::set_has_no_message_loop() _before_ calling
	space_input::open().

	If there are problems please read the section Trouble Shooting or contact the author 

	\author Joachim Staib (s2458101@mail.inf.tu-dresden.de)

 \section example_sec Examples
 	Using space_input is fairly easy. Generally you have to do 5 steps:
		- create a new instance of the space_input class
		- call space_input::open(CONFIG_NAME) to open/initialize the device
		- call space_input::grab_focus() to grab the focus for this instance
		- get information by using methods like get_transformation or get_keys
		- after finishing work close the device by calling space_pilot::close or by just destroying the instance

	Here is a small code example
	\code
	#include <iostream>
	#include "space_input.h"

	int main() 
	{
		space_input *test_input = new space_input();
		double translation;

		// We have no message loop. You only need to
		// call this in such a case.
		test_input->set_has_no_message_loop();

		// Open the device
		test_input->open("MyConfiguration");

		// Grab focus
		test_input->grab_focus();

		// Get the translation
		test_input->get_translation(translation);
		std::cout<<"Translation is: "<<translation[0]<<", "
			 <<translation[1]<<", "
			 <<translation[2]<<"\n";

		// Destroy the instance
		delete test_input;
	}.
	\endcode

	This example shows the basic access to the space-device information. To time information retrieval (i.e. only update
	a certain program feature when the user does something on the device) you can use callbacks. There are two possibilities:
		- Make a child class of space_input and overwrite the methods on_sensor, on_keyup, on_keydown
		- Attach own callbacks by using attach_callback. See the documentation of this method for more details and links.

	Included with this library are some more complete examples that compile and demonstrate the functionality and paradigma
	of the usage of the information provided by the device:
		- ConsoleTest: A simple console based test that demonstrates how to read information and how to subclass space_input
		- CallbackTest: Another console based test that does callbacking by using attach_callback
		- SceneTest: A more elaborated test that uses OpenGL and fltk2 to display a 3D cube that can be moved with the space-device
		- MultiFocusTest: A test that uses OpenGL and fltk2 to display two cubes in two different canvases that can be switched
		  by clicking the mouse on one of them. This demonstrates the above mentioned multi-instance ability.
  
 
 \section troubleshoot_sec Trouble Shooting
 
	\subsection help1 Calling space_input::open returns false! 
		If this happens something went wrong during initialization. To get a more precise description of what didn't work you can 
		call space_input::get_error_code. This returns one element of the space_input_error enum (which is the same as the
		space_pilot_error enum). The reported codes can mean the following:

		\subsubsection windows Unter Windows
			- SPERROR_NODEVICE:
			  In the initialization step the lib queries the COM interface
			  for something that provides an appropriate device. This normally only fails
			  when the 3dxWare-driver isn't activated (or when you have a serious problem
			  with the windows COM interface).
			- SPERROR_NOTSETUP:
			  This error means that your operating system has a problem with the allocation
			  of the resources needed to communicate to the device. The following problems 
			  can cause these errors:
				- Initialization of the COM framework failed. For every thread that uses the Windows COM
				  interface an initialization has to be made. There are two different modes for this
				  initialization where only one can apply for one thread at a time. Normally this means 
				  that some other method in the running thread has initialized the COM framework in another 
				  mode.Try calling space_input::set_has_no_message_loop() before calling open 
				  (or vice versa: remove this line if it's in your code).
				- No instance of the device interface could be created. This happens if the COM framework
				  doesn't contain an entry for the 3DxWare device. There is nothing much that can be done
				  here except reinstalling 3DxWare and hoping that things work.

		\subsubsection linux Under Linux
			- SPERROR_NODEVICE:
			  Under Linux the clients that want to use functionality from 3dDxWare connect a window to the driver
			  to which then events are injected. In the X-window system each event has a certain identifier (called
			  atom). SPERROR_NODEVICE is returned if one of these atoms couldn't be found which means that the
			  3DxWare didn't register one of the required event types for the communications. This normally means
			  that the driver isn't activated or that there is any other communication error with the X system.

	\subsection help2 Polling the device gives zeros / Callbacks are not triggered
		This means that the information isn't retrieved from the device. To fix this error try the following:
			- You probably forgot to call space_input::grab_focus() on the instance you are polling from
			- print the result of space_input::open() via space_input::get_error_code and see the section
			  above.
			- If you don't have a message loop in your application (which means you are not using some toolkit
			  like Win32 (or MFC), GTK, QT, fltk, Motiv ...) you must decouple information update from the
			  loop by calling space_input::set_has_no_message_loop() before calling space_input::open;

	\subsection help3 There are XBadWindow and synchronization errors or the program crashes
		Under linux it is not possible to serialize the callbacking in the message loop (because the X-system
		is the abstraction layer that introduces events without the possiblility for callbacks). Although the
		information that is provided by the class is thread save (by using locking) you have to make sure that
		your code doesn't run into concurrency problems. Good ways to avoid these problems are:
			- Init multithreading stuff in your toolkit. Normally every better toolkit has some method
			  to prepare the toolkit for multithreading (where synchronization is done automatically).
			  In gtk2 for example one can call gdk_threads_init() on initialization and gdk_threads_enter()
			  when something graphical shall be done in another thread.
			- Don't do any drawing at all in the callback function.

	If there are any further problems please contact the author.
 */
#pragma once

#include <iostream>
#include "space_pilot.h"

#include "lib_begin.h"

/**
 \defgroup Enums Structs, Enums and Typedefs
*/
/*@{*/
/** \typedef struct space_pilot_info space_input_info;
  Information on various states of the device.
  This is just a renaming of the space_pilot_info structure defined in spacepilot.h
  It contains:
 	- rot: a double[4] containing the rotation axis (0-2) and the angle in degrees
 	- pos: a double[3] containing the translation 
 	- period: a double for the period
 	- keys: an unsigned int representing the bitmask of pressed keys
*/
typedef space_pilot_info space_input_info;

/** \typedef space_pilot_event space_input_event
  Designators to attach a callback.
  This is just a renaming of the space_pilot_event enum defined in spacepilot.h. See
  the documentation for spacepilot.h for more information
  Possible (OR-combinable) values are:
 	- SPE_NONE: Trigger on no event
 	- SPE_ROTATION: Trigger on rotation events
 	- SPE_TRANSLATION: Trigger on translation events
 	- SPE_KEYDOWN: Trigger on key down events
 	- SPE_KEYUP: Trigger on key up events
 	- SPE_DEVICECHANGE: Trigger, when the device was changed
 	- SPE_ALL: Trigger on all events
*/
typedef space_pilot_event space_input_event;

/** \typedef space_pilot_error space_input_error 
  Designators for device errors.
  This is just a renaming of the space_pilot_error enum defined in spacepilot.h. See
  the documentation for spacepilot.h for more information
  Possible values are:
 	- SPERROR_NONE: No error
 	- SPERROR_NODEVICE: No device was found
 	- SPERROR_NOCONNECTION: A device was found but no connection was possible
 	- SPERROR_NOTREADY: A device is present and connected but it doesn't react
 	- SPERROR_NOTSETUP: The system isn't set up to talk with the device.
  See the troubleshooting section for more information on these errors.
 */
typedef space_pilot_error space_input_error;

/** \typedef space_pilot_callback space_input_callback
  The callback function.
  This is just a renaming of the space_pilot_callback prototype defined in spacepilot.h. See
  the documentation for spacepilot.h for more information
  @param event a bitmask of the event, that occured
  @param info a pointer to a space_input_info struct that contains updated values. Contents must not be changed.
  @param user_data special user data the user defined when adding the callback
*/
typedef space_pilot_callback space_input_callback;
/*@}*/


/**
 communication with 3DxWare driven devices.
 This class provides a c++ wrapper for the c-like functions defined in spacepilot.h for
 communicating with a 3DxWare-device (SpacePilot, SpaceMouse, SpaceTraveller ...). 

 The methods in this class combine functionality for getting information using polling or 
 various ways to use a callbacking system.
 Polling can be done by using methods like space_input::get_transformation or 
 space_input::get_keys. 
 For callbacking there exist two ways:
	- Create a child class that overwrite the virtual methods on_sensor, on_keydown and on_keyup.
      	  Callbacking is enabled automatically for these methods
	- Hook own event handlers to a more fine granulated selection of possible events by
	  using space_input::attach_callback and space_input::remove_callback. Besides giving you
	  the possiblility to hook to more specific events you can attach as many callbacks (which 
	  follow the prototype space_input_callback) as you want.

	For problems see the troubleshooting section.
*/
class CGV_API space_input {
public:
	// Constructor
	space_input();
	// Destructor
	virtual ~space_input();


	/**	Opens this space pilot connection. 
		This method tries to open a connection to the space pilot dispatcher and initializes
		the device for the given configuration.
		@param config_name A configuration name
		@returns true if successful
	*/
	bool open(const char* config_name);

	/**	Closes the space pilot connection.*/
	void close();

	/**	Tries to grab the pilots focus. 
		Using this method you can force the space pilot to give focus to this instance. It temporally
		disconnects all other handles. */
	void grab_focus();

	/** Releases focus.
		When you want to disable the space pilot functionality for this instance you can
		call this method. */
	void release_focus();

	/** Call this method if your application doesn't have an own message loop. */
	void set_has_no_message_loop();



	/** Attaches a callback. 
		Using callbacks can be done in two ways: By inheriting this class and overwriting
		on_sensor, on_keydown and on_keyup or by registering own callbacks from the form
		static void callback_function(int event_mask, space_input_info* info, void* user_data);
		see the space pilot low level documentation for more information on this. This method
		just wraps the functionality. 
		@param mask an ORed mask of elements from space_input_event
		@param callback callback function to attach 
		@param user_data user data to provide for the callback function */
	void attach_callback(int mask, space_input_callback callback, void* user_data);

	/** Detaches a callback.
		This method detaches a callback. See attach_callback for more information about this
		way of using callbacks.
		@param callback callback function to detach
		@param user_data user data that was provided when attaching this callback */
	void detach_callback(space_pilot_callback callback, void* user_data);

	/** Disables certain events.
		Disables callbacks for specific types of events. This is a wrapper for 
		space_pilot_disable_events from the low level API.
		@param mask a bitmask of events to disable */
	void disable_events(int mask);

	/** Enable/disable sensor input events.
		To temporally enable/disable sensor events (translation/rotation) for callbacks
		you can use this method.
		@param state true to enable sensor events */
	void set_sensor_enabled(bool state);

	/** Enable/disable keyboard input events.
		To temporally enable/disable keyboard events for callbacks use this method. Use that
		only the keys that are free programmable (set on "Button X"/"Taste X") are affected. */
	void set_keyboard_enabled(bool state);

	/** Method to execute on sensor input.
		Overwrite this method to capture sensor input. An alternative is to attach an own callback.
		See attach_callback for more details. */
	virtual void on_sensor();

	/** Method to execute on keydown events.
		Overwrite this method to capture keydown events. An alternative is to attach an own callback.
		@see attach_callback */
	virtual void on_keydown();

	/** Method to execute on keydown events.
		Overwrite this method to capture keyup events. An alternative is to attach an own callback.
		@see attach_callback */
	virtual void on_keyup();

	/** Gets the rotation angle.
		You can get the rotation by an rotation axis (see get_rotation_axis) and 
		the rotation angle. This method returns the angle.
		@returns the rotation angle */
	double get_angle();

	/** Gets the rotation axis. 
		@param dest a double[3] to store the axis in
		@see get_angle */
	void get_rotation_axis(double *dest);

	/** Gets the period. 
		Returns the time in milliseconds the device information is polled
		@returns the period */
	double get_period();

	/** Gets the button state.
		Returns a bitmask that represents the pressed button. That means, button1 is pressed,
		if the first bit is set (keys&1>0) and so on. This method is quite intelligent to
		avoid missed buttons - find out what that means :).
		@returns a buttons bitmask */
	unsigned int get_keys();

	/** Gets the translation.
		This is a convenience function to get the whole translation information on one call.
		@param dest a double[3] for the translation vector */
	void get_translation(double *dest);

	/** Gets the transformation.
		This is a convenience function to get the whole transformation of the device on one call. 
		@param dest_trans a double[3] for the translation vector
		@param dest_rot a double[4] for the rotation axis (first 3) and the angle
		@param dest_period the period (see get_period) */
	void get_transformation(double *dest_trans, double *dest_rot, double *dest_period);

	/** Get the last error.
		Gets the error of the last operation.
		@returns the last error */
	static unsigned int get_error_code();


private:
	int handle;
	

	static void cb_on_sensor(int event, space_input_info* info, space_input *self);
	static void cb_on_keyup(int event, space_input_info* info, space_input *self);
	static void cb_on_keydown(int event, space_input_info* info, space_input *self);
};

#include <cgv/config/lib_end.h>
