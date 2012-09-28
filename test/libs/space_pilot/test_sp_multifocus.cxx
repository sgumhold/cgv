/*
 Space_input example: MultiFocusTest
 This example demonstrates (besides the things that are also
 contained in the SceneTest example) the use of multiple instances
 of space_input. Having more than one instance is useful if you have
 multiple contexts where you want to use space pilot information.
 Only one instance can have the focus at a time. An instance can call
 grab_focus() to make itself active (and deactivate all other instances).
 Also it is possible for each instance to have an own configuration name
 which allows for more than one configuration (in the 3dxware-panel) for
 one application.
 See the comments in multiviewer.h for more details on the program structure.
*/

#include <fltk/run.h>
#include "sp_multiviewer.h"
#include <iostream>

int main()
{
	multi_viewer *wnd = new multi_viewer(800, 400);

	wnd->show();
	return run();
}
