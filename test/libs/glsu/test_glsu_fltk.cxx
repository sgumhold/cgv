#include <fltk/GlWindow.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/gl.h>
#include "test_glsu.h"

using namespace fltk;

class GlsuTestWindow : public GlWindow, public test_glsu
{
public:
	GlsuTestWindow(int w, int h) : GlWindow(w,h,"test_glsu_fltk")
	{
		set_aspect((GLdouble)w/h);
		add_timeout(0.02f);
	}
	int handle(int e)
	{
		switch (e) {
		case TIMEOUT :
			if (step_animation())
				redraw();
			repeat_timeout(0.02f);
			return true;
		case ENTER :
		case FOCUS :
			return true;
		case MOVE :
			{
				bool res = false;
				float x = (float)(event_x() - w()/2)/(w()/2);
				float y = (float)(h()/2 - event_y())/(h()/2);
				if (event_key_state(LeftShiftKey) || event_key_state(RightShiftKey) || (event_state()&CAPSLOCK))
					res = mouse_event(x, y, 0);
				else if (event_key_state(LeftAltKey) || event_key_state(RightAltKey))
					res = mouse_event(x, y, 1);
				else if (event_key_state(LeftCtrlKey) || event_key_state(RightCtrlKey))
					res = mouse_event(x, y, 2);
				if (res) {
					redraw();
					return true;
				}
				break;
			}
		case MOUSEWHEEL : 
			if (wheel_event(0.1f*event_dy())) {
				redraw();
				return true;
			}
			break;
		case KEY: {
			bool res = false;
			switch (event_key()) {
			case SpaceKey : res = key_event(Key_SPACE); break;
			case ReturnKey: res = key_event(Key_ENTER); break;
			case EscapeKey: res = key_event(Key_ESCAPE); break;
			case F4Key  : res = key_event(Key_F4); break;
			case F5Key  : res = key_event(Key_F5); break;
			case F6Key  : res = key_event(Key_F6); break;
			case F7Key  : res = key_event(Key_F7); break;
			case F10Key : res = key_event(Key_F10); break;
			}
			if (res) {
				redraw();
				return true;
			}
			break;
				  }
		}
		return Widget::handle(e);
	}
	// redraws the window
	void draw()
	{
		if (!valid())
			test_glsu::init();
		test_glsu::draw();
	}
};



int main()
{
	GlsuTestWindow w(1024, 768);
	w.show_help();
	w.show();
	return run();
}
