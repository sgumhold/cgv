#pragma once

#include <fltk/Window.h>
#include <fltk/Widget.h>
#include <fltk/draw.h>
#include "sp_sceneviewer.h"

using namespace fltk;

/*
 class multi_viewer:
 This class represents a fltk::Window that contains two
 openGL views with a cube (which are the same as in the SceneTest
 example). It hooks on the callbacks for the two views (which get
 evoked when the user presses the mouse button on it) and sets
 the input focus according to the active view (that is done in
 multi_viewer::view1_callback and multi_viewer::view2_callback.

 Also there is a method "draw" that overwrites the standard draw
 method of the window to stroke a rectangle around the two views
 to indicate the active one.
*/
class multi_viewer: public Window {
public:
	multi_viewer(int w, int h);
	~multi_viewer();

	// called on draw updates. draws rectangles around the two views
	void draw();

private:
	// the two views
	scene_viewer* view[2];
	// a toggle to store the active view
	bool first_active;

	// callback functions that are triggered when the user clicks
	// on one of the views.
	static void view1_callback(Widget *w, multi_viewer *self);
	static void view2_callback(Widget *w, multi_viewer *self);
};
