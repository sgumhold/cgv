/*
 Space_input example: SceneTest
 The purpose of this example is to show how to work with the 
 information the space device provides. It shows an openGL
 drawn cube in an fltk2 window which can be translated and
 rotated with the space pilot. also, pressing the FIT button
 puts the cube back to its initial position. the real interesting
 part of this example is the method sceneviewer::on_sensor where
 the calculation is shown and explained.
 see the comment in sceneviewer.h for more details on this example.

*/

#include <fltk/run.h>
#include "sp_sceneviewer.h"
#include <iostream>

int main()
{
	scene_viewer *wnd = new scene_viewer(100, 100, 600,600);

	// open the Space Pilot
	if (!wnd->open("SceneDemo")) {
		printf("Couldn't open space pilot\n");
		return 0;
	}

	// grab focus
	wnd->grab_focus();

	wnd->show();
	return run();
}
