#include <iostream>
#ifdef WIN32
#include <windows.h>				// For "Sleep(...)"
#endif
#include <space_input.h>

using namespace std;


static void on_sensor_callback(int event_mask, space_input_info* info, void* user_data)
{
	cout<<"Sensor event: ";

	if ((event_mask & SPE_TRANSLATION))
		cout<<"trans("<<info->pos[0]<<", "<<info->pos[1]<<", "<<info->pos[2]<<") ";

	if ((event_mask & SPE_ROTATION))
		cout<<"rot("<<info->rot[0]<<", "<<info->rot[1]<<", "<<info->rot[2]<<"; "<<info->rot[3]<<")";

	cout<<"\n";
}



static void on_key_callback(int event_mask, space_input_info* info, void* user_data)
{
	if (event_mask & SPE_KEYDOWN)
		cout<<"Key down event ("<<info->keys<<")\n";
	else
		cout<<"Key up event\n";
}




int main(int argc, char* argv[])
{
	space_input *input = new space_input();

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

	// attach a callback for sensor events
	input->attach_callback(SPE_TRANSLATION | SPE_ROTATION, 
						   (space_input_callback)on_sensor_callback, NULL);

	// attach a callback for key events
	input->attach_callback(SPE_KEYUP | SPE_KEYDOWN,
						   (space_input_callback)on_key_callback, NULL);


	// get focus for this instance
	input->grab_focus();

	std::cout<<"Done! Do something on your SpacePilot\n";

	while(1) {
		// Nothing to do here since we use callbacks. Anyway, call 
		// sleeps to make the program not use 100% CPU
		#ifdef WIN32
		Sleep(10000);
		#else
		usleep(10000);
		#endif
	}


	// delete the instance. The destructor checks whether the device
	// is already closed and does this for you if not. To close the device
	// without deleting the instance you can call space_input::close()
	delete input;

	return 0;
}

