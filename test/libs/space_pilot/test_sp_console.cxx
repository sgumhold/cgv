/*
 Space_input example: ConsoleTest.
 This test shows how to use the features of the space pilot without having
 a window or an own message loop. Here, a new class called my_space_input
 is created that inherits the functionality of space_input and overwrites
 the methods for sensor and key callbacks (in the space_input these are only
 dummies with the purpose of overwriting). The interesting part of this small
 test is how to set up the API if it shall be used without a message loop.
 For this, the method space_input::set_has_no_message_loop() is called BEFORE
 space_input::open(). The order is important here.
*/
#include <iostream>
#ifdef WIN32
#include <windows.h>				// For "Sleep(...)"
#endif
#include <space_input.h>

#ifdef WIN32
[module(name="SpSample")]
#endif

// make a small child of space_input that overwrites the callback functions
class my_space_input: public space_input {
public:

	// sensor event callback
	void on_sensor() {
		double trans[3], rot[4], period;

		// read the transformation
		get_transformation(trans, rot, &period);

		std::cout<<"Sensor event: trans("<<trans[0]<<", "<<trans[1]<<", "<<trans[2]<<") "
			<<"rot("<<rot[0]<<", "<<rot[1]<<", "<<rot[2]<<"; "<<rot[3]<<")\n";
	}

	// keydown event callback
	void on_keydown() {
		std::cout<<"Key Down ("<<get_keys()<<")\n";
	}

	// keyup event callback
	void on_keyup() {
		std::cout<<"Key up\n";
	}
};



int main(int argc, char* argv[])
{
	space_input *input = new my_space_input();

	// there is no message loop! call this BEFORE calling space_input::open().
	// also this only has to be called once if you use multiple instances of
	// the space_input class.
	input->set_has_no_message_loop();

	std::cout<<"Opening Device ...\n";

	// init and open device with the configuration name "Sample"
	if (!input->open("Sample")) {
		std::cout<<" no luck - please check whether the driver is running. sorry\n";
		return 0;
	}

	// get focus for this instance
	input->grab_focus();

	std::cout<<"Done! Do something on your SpacePilot\n";

	while(1) {
		// Nothing to do here since we use callbacks. Anyway, call 
		// sleeps to make the program not use 100% CPU
		#ifdef WIN32
		Sleep(10000);
		#else
		usleep(100000);
		#endif
	}


	// delete the instance. The destructor checks whether the device
	// is already closed and does this for you if not. To close the device
	// without deleting the instance you can call space_input::close()
	delete input;

	return 0;
}

