#include <fltk/GlWindow.h>
#include <fltk/Cursor.h>
#include <fltk/run.h>
#include <fltk/events.h>
#include <windows.h>
#include <GL/gl.h>
#include "sample_application.h"
#include "multi_mouse_fltk.h"

using namespace fltk;
using namespace std;
using namespace cgv::gui;

struct MultiMouseWindow : public GlWindow, public multi_mouse_handler, public sample_application
{
	/// in case of animated cursor, continuously post redraw
	static void timer_event(void* This)
	{
		MultiMouseWindow* mmw = (MultiMouseWindow*)This;
		if (mmw->c.is_created() && mmw->c.get_nr_steps() > 0)
			mmw->redraw();
		fltk::repeat_timeout(1.0f/60,timer_event,mmw); 
	}	
	/// constructors
	MultiMouseWindow(int w, int h) : GlWindow(w,h,"Multi Mouse Demo") { fltk::add_timeout(1.0f/60,timer_event,this); }
	MultiMouseWindow(int x, int y, int w, int h) : GlWindow(x,y,w,h,"Multi Mouse Demo")	{ fltk::add_timeout(1.0f/60,timer_event,this); }
	/// called when a new mouse device is attached (attach=true) or an existing is detached (attach=false)
	void on_mouse_device_change(bool attach, void* id)
	{
		process_mouse_change_event(attach, id, w(), h());
		redraw();
	}
	bool handle_multi_mouse(const multi_mouse_event& mme)
	{
		bool res = handle_mouse_event(mme);
		redraw();
		return res;
	}
	// hide regular mouse pointer
	int handle(int event)
	{
		// hide regular mouse cursor
		if (event == fltk::ENTER)
			cursor(CURSOR_NONE);
		return GlWindow::handle(event);
	}
	void draw()
	{	
		// clear gl window
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		// configure to mouse coordinates
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,w(),h(),0,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// draw sample application
		draw_scene(fltk::get_time_secs());
	}
};

#define FULLSCREEN

int main(int argc, char** argv)
{
#ifdef FULLSCREEN
	// create application window and switch to fullscreen
	MultiMouseWindow *window = new MultiMouseWindow(800, 600);
	window->show(argc, argv);
	window->fullscreen();
#else
	// create another application window and add opengl window as child
	Window* w = new Window(800,600,"test");
	w->begin();
	MultiMouseWindow *window = new MultiMouseWindow(0,0,400, 600);
	w->end();
	w->show(argc, argv);
#endif
	// the following registration and initialization code can be done in arbitrary order
	window->init_mice(window->w(), window->h());
	register_multi_mouse_handler(window);
	enable_multi_mouse();
	attach_multi_mouse_to_fltk(window);

	// start message loop processing
	return run();
}
