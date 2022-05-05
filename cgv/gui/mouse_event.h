#pragma once

#include "event.h"

#include "lib_begin.h"

namespace cgv {
	namespace gui {


/// different actions that a mouse can perform
enum MouseAction { 
	MA_PRESS,    //!< mouse button pressed
	MA_RELEASE,  //!< mouse button released
	MA_WHEEL,    //!< mouse wheel moved
	MA_MOVE,     //!< mouse pointer moved
	MA_DRAG,     //!< mouse drag action
	MA_ENTER,    //!< mouse enter window action
	MA_LEAVE     //!< mouse leave window action
};

/// different mouse buttons that can be ored together to specify the button state
enum MouseButton 
{ 
	MB_NO_BUTTON = 0,      //!< no button
	MB_LEFT_BUTTON = 1,    //!< left button
	MB_MIDDLE_BUTTON = 2,  //!< middle button
	MB_RIGHT_BUTTON = 4    //!< right button
};

/// class to represent all possible mouse events with the EID_MOUSE
class CGV_API mouse_event : public event
{
protected:
	/// x position of mouse pointer
	short x;
	/// y position of mouse pointer
	short y;
	/// change in x position
	short dx;
	/// change in y position
	short dy;
	/// store MouseAction 
	unsigned char action;
	/// store the button state
	unsigned char button_state;
	/// store the pressed button
	unsigned char button;
	/// store mouse event flags
	unsigned char flags;
	/// the texted resulting from a drag and drop event
	std::string dnd_text;
public:
	/// construct a mouse
	mouse_event(int x, int y, MouseAction _action, unsigned char _button_state = 0, unsigned char _button = 0, short _dx = 0, short _dy = 0, unsigned char _modifiers = 0, unsigned char _toggle_keys = 0, double _time = 0);
	/// write to stream
	void stream_out(std::ostream& os) const;
	/// read from stream
	void stream_in(std::istream& is);
	/// current mouse x position (origin is top-left of window)
	short get_x() const;
	/// current mouse y position (origin is top-left of window)
	short get_y() const;
	/// for move and drag events the difference in x to the previous position
	short get_dx() const;
	/// for move and drag events the difference in y to the previous position, for wheel events the amount the wheel has changed
	short get_dy() const;
	/// return the mouse action
	MouseAction get_action() const;
	/// return the %button state as values from MouseButton combined with a logical or-operation
	unsigned char get_button_state() const;
	/// return the pressed or released %button for a %button press or release action
	unsigned char get_button() const;
	/// only valid in a MA_RELEASE event with the flag MF_DND set, return the text resulting from the drag&drop action
	const std::string& get_dnd_text() const;

	/// set current mouse x position 
	void set_x(short _x);
	/// set current mouse y position 
	void set_y(short _y);
	/// set for move and drag events the difference in x to the previous position
	void set_dx(short _dx);
	/// set for move and drag events the difference in y to the previous position, for wheel events the amount the wheel has changed
	void set_dy(short _dy);
	/// return the mouse action
	void set_action(MouseAction _action);
	/// set the %button state as values from MouseButton combined with a logical or-operation
	void set_button_state(unsigned char _button_state);
	/// set the pressed or released %button for a %button press or release action
	void set_button(unsigned char _button);
	/// set the drag&drop text
	void set_dnd_text(const std::string& text);
};

	}
}

#include <cgv/config/lib_end.h>