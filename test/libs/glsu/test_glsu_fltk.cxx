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
		case KEY: {
			bool res = false;
			switch (event_key()) {
			case SpaceKey : res = key_event(Key_SPACE); break;
			case EscapeKey: res = key_event(Key_ESCAPE); break;
			case F4Key  : res = key_event(Key_F4); break;
			case F5Key  : res = key_event(Key_F5); break;
			case F6Key  : res = key_event(Key_F6); break;
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
