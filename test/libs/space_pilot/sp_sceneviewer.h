#ifdef _WIN32
#pragma once
#endif

#ifndef SCENEVIEWER_H
#define SCENEVIEWER_H

#include <fltk/GlWindow.h>
#include <fltk/events.h>
#include <fltk/run.h>
#include <fltk/gl.h>
#include <GL/glu.h>
#include <space_input.h>
#include <math.h>

using namespace fltk;

/*
 class scene_viewer:
 This is the main class (and the only one) in this example. 
 it inherits the functionality of a GlWindow to be able to
 draw itself and also the functionality of space_input for
 controlling. Whenever the device is moved the methods
 on_sensor (which is an overwritten method) is called. 
 This method then gets all neccessary information (translation,
 rotation axis, rotation angle ...) and constructs a new 
 transformation matrix (stored in "transform"). For this the
 helper functions matrix_reset, matrix_multiply and matrix_rotation
 are used.
 The method "draw()" is called automatically whenever the window needs
 to be redrawed but also every 1/30 seconds, which is made using 
 fltk::add_timeout (in the constructor of scene_viewer). here the
 transformation matrix is set to be the openGL modelview matrix and
 then the cube is drawn. 
*/
class scene_viewer: public GlWindow, public space_input {
public:
	scene_viewer(int x, int y, int w, int h);

	~scene_viewer();

	// redraws the window
	void draw();
	// handles events from the window
	int handle(int event);

	// called on sensor updates
	void on_sensor();
	// called on keydown events
	void on_keydown();

private:
	// the transformation matrix
	double transform[4*4];
	// colors for every face of the cube
	double *colors[6];
	// the translation reported from the device
	double cur_position[3];
	// the last time the transformation was updated
	double last_time;

	// configures the openGL context
	void configure_gl();
	// clears transform and cur_position to the initial position
	void set_initial_position();


	// makes a 4x4 matrix the identity matrix
	void matrix_reset(double *matrix);
	// multiplies matrix m1 and m2 and store the result in dest
	void matrix_multiply(double *dest, double *m1, double *m2);
	// creates a rotation matrix from a rotation axis and a rotation angle
	// and stores the result in dest
	void matrix_rotation(double *dest, double *axis, double angle);
};


// some colors
const GLfloat col_black[4] = { 0.0, 0.0, 0.0, 1 };
const GLfloat col_white[4] = { 1.0, 1.0, 1.0, 1 };

#endif
