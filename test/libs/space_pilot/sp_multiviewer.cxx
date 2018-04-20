#include "sp_multiviewer.h"

multi_viewer::multi_viewer(int w, int h): Window(100, 100, w, h, "MultiFocus Demo") 
{
	// add the two views
	begin();
		view[0] = new scene_viewer(5, 5, w/2-10, h-10);
		view[1] = new scene_viewer(w/2+5, 5, w/2-10, h-10);
	end();

	// set a callback function that will be called when the user presses
	// on one of the openGL canvases
	view[0]->callback((Callback*)view1_callback, this);
	view[1]->callback((Callback*)view2_callback, this);

	// open the two instances for the device
	view[0]->open("SampleView1");
	view[1]->open("SampleView2");

	// grab focus for the first instance
	view[0]->grab_focus();
	first_active = true;
}


multi_viewer::~multi_viewer() 
{
	delete view[0];
	delete view[1];
}



void multi_viewer::draw() 
{
	// first, draw everything that is important
	Window::draw();

	// now according to the variable first_active (that is true when the
	// first canvas is active, false otherwise) draw a red or a gray
	// rectangle around the two canvases
	first_active? setcolor(fltk::color(255, 0, 0)): setcolor(fltk::color(100, 100, 100));
	strokerect(3, 3, w()/2-6, h()-6);
	first_active? setcolor(fltk::color(100, 100, 100)): setcolor(fltk::color(255, 0, 0));
	strokerect(w()/2+3, 3, w()/2-6, h()-6);
}



void multi_viewer::view1_callback(Widget *w, multi_viewer *self)
{
	// the user clicked on the left canvas. grab focus for the space_input instance
	// and mark it as active
	self->first_active = true;
	self->view[0]->grab_focus();
	self->redraw();
}



void multi_viewer::view2_callback(Widget *w, multi_viewer *self)
{
	// the user clicked on the right canvas. grab focus for the space_input instance
	// and mark the left canvas as inactive
	self->first_active = false;
	self->view[1]->grab_focus();
	self->redraw();
}
