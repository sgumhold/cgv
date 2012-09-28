#include <cgv/gui/mouse_event.h>

namespace cgv {
	namespace gui {


// construct a key event from its textual description 
mouse_event::mouse_event(int _x, int _y, MouseAction _action, unsigned char _button_state, unsigned char _button, short _dx, short _dy, unsigned char _modifiers, unsigned char _toggle_keys, double _time)
	: event(EID_MOUSE,_modifiers,_toggle_keys,_time), 
	  x(_x), y(_y), dx(_dx), dy(_dy), action(_action), button_state(_button_state), button(_button)
{
}
// write to stream
void mouse_event::stream_out(std::ostream& os) const
{
	const char* action_strs[] = {
		"press", "release", "wheel", "move", "drag", "enter", "leave"
	};
	const char* button_strs[] = {
		"", "Left_Button", "Middle_Button", "", "Right_Button"
	};
	event::stream_out(os);
	if ((get_button_state()&MB_LEFT_BUTTON)!=0)
		os << "Left_Button+";
	if ((get_button_state()&MB_MIDDLE_BUTTON)!=0)
		os << "Middle_Button+";
	if ((get_button_state()&MB_RIGHT_BUTTON)!=0)
		os << "Right_Button+";
	os << "mouse " << action_strs[get_action()];
	if (get_action() == MA_PRESS || get_action() == MA_RELEASE)
		os << " " << button_strs[get_button()];
	if (get_action() == MA_WHEEL)
		os << "(" << get_dy() << ")";
	os << " at " << get_x() << "," << get_y();
	if (get_action() == MA_DRAG || get_action() == MA_MOVE) 
		os << "(" << get_dx() << "," << get_dy() << ")";
}
// read from stream
void mouse_event::stream_in(std::istream&)
{
}
// 
short mouse_event::get_x() const
{
	return x;
}
// 
short mouse_event::get_y() const
{
	return y;
}
// not used yet
short mouse_event::get_dx() const
{
	return dx;
}
// used for mouse wheel delta
short mouse_event::get_dy() const
{
	return dy;
}
// 
MouseAction mouse_event::get_action() const
{
	return (MouseAction)action;
}
// 
unsigned char mouse_event::get_button_state() const
{
	return button_state;
}
// 
unsigned char mouse_event::get_button() const
{
	return button;
}

/// set current mouse x position 
void mouse_event::set_x(short _x)
{
	x = _x;
}

/// set current mouse y position 
void mouse_event::set_y(short _y)
{
	y = _y;
}

/// set for move and drag events the difference in x to the previous position
void mouse_event::set_dx(short _dx)
{
	dx = _dx;
}

/// set for move and drag events the difference in y to the previous position, for wheel events the amount the wheel has changed
void mouse_event::set_dy(short _dy)
{
	dy = _dy;
}

/// return the mouse action
void mouse_event::set_action(MouseAction _action)
{
	action = _action;
}

/// set the %button state as values from MouseButton combined with a logical or-operation
void mouse_event::set_button_state(unsigned char _button_state)
{
	button_state = _button_state;
}

/// set the pressed or released %button for a %button press or release action
void mouse_event::set_button(unsigned char _button)
{
	button = _button;
}


	}
}
