#include "space_input.h"

space_input::space_input() {
	handle = -1;
}


space_input::~space_input() {
	if (handle != -1)
		space_pilot_close(handle);
}



// Open a space pilot connection
bool space_input::open(const char* config_name) {
	// try to init the device
	if (!space_pilot_init())
		return false;

	handle = space_pilot_open(config_name);

	if (handle != -1) {
		space_pilot_add_callback(handle, 
								 SPE_ROTATION | SPE_TRANSLATION, 
								 (space_pilot_callback)this->cb_on_sensor, 
								 this);
		space_pilot_add_callback(handle,
								 SPE_KEYDOWN,
								 (space_pilot_callback)this->cb_on_keydown,
								 this);
		space_pilot_add_callback(handle,
								 SPE_KEYUP,
								 (space_pilot_callback)this->cb_on_keyup,
								 this);
		return true;
	}

	return false;
}



// Closes the space pilot connection.
void space_input::close() {
	space_pilot_close(handle);
	handle = -1;
}


void space_input::set_has_no_message_loop()  {
	space_pilot_set_no_message_loop();
}



// Tries to grab the pilots focus. 
void space_input::grab_focus() {
	space_pilot_grab_focus(handle);
}



// Releases focus.
void space_input::release_focus() {
	space_pilot_release_focus();
}



// Attaches a callback. 
void space_input::attach_callback(int mask, space_pilot_callback callback, void* user_data) {
	space_pilot_add_callback(handle, mask, callback, user_data);
}



// Detaches a callback.
void space_input::detach_callback(space_pilot_callback callback, void* user_data) {
	space_pilot_remove_callback(handle, callback, user_data);
}



// Disables certain events.
void space_input::disable_events(int mask) {
	space_pilot_disable_events(handle, mask);
}



// Enable/disable sensor input events.
void space_input::set_sensor_enabled(bool state) {
	if (state)
		space_pilot_disable_events(handle, 
								   ~(space_pilot_get_enabled_events(handle) | 
								   SPE_TRANSLATION | 
								   SPE_ROTATION));
	else
		space_pilot_disable_events(handle, SPE_TRANSLATION | SPE_ROTATION);
}



// Enable/disable keyboard input events.
void space_input::set_keyboard_enabled(bool state) {
	if (state)
		space_pilot_disable_events(handle,
									~(space_pilot_get_enabled_events(handle) |
									SPE_KEYDOWN |
									SPE_KEYUP));
	else
		space_pilot_disable_events(handle, SPE_KEYDOWN | SPE_KEYUP);

}



// Method to execute on sensor input.
void space_input::on_sensor() {
	// do something here
}



// Method to execute on keydown events.
void space_input::on_keydown() {
	// do something here
}



// Method to execute on keydown events.
void space_input::on_keyup() {
	// do something here
}



// Gets the rotation angle.
double space_input::get_angle() {
	double trans[3], rot[4];
	space_pilot_poll_motion(trans, rot);

	return rot[3];
}



// Gets the rotation axis.
void space_input::get_rotation_axis(double *dest) {
	double trans[3], rot[4];
	space_pilot_poll_motion(trans, rot);

	memcpy(dest, rot, sizeof(double)*3);
}



// Gets the period. 
double space_input::get_period() {
	return space_pilot_poll_period();
}



// Gets the button state.
unsigned int space_input::get_keys() {
	return space_pilot_poll_keys();
}



// Gets the translation.
void space_input::get_translation(double *dest) {
	double rot[4];
	space_pilot_poll_motion(dest, rot);
}



// Gets the transformation.
void space_input::get_transformation(double *dest_trans, double *dest_rot, double *dest_period) {
	space_pilot_poll_motion(dest_trans, dest_rot);
	*dest_period = space_pilot_poll_period();
}



void space_input::cb_on_sensor(int event, space_input_info* info, space_input *self) {
	self->on_sensor();
}



void space_input::cb_on_keyup(int event, space_input_info* info, space_input *self) {
	self->on_keyup();
}



void space_input::cb_on_keydown(int event, space_input_info* info, space_input *self) {
	self->on_keydown();
}


unsigned int space_input::get_error_code() {
	return space_pilot_get_error();
}

