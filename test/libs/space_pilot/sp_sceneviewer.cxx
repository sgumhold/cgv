#include "sp_sceneviewer.h"
#include <iostream>
#include <stdarg.h>
#include <time.h>


scene_viewer::scene_viewer(int x, int y, int  w, int h): GlWindow(x, y, w, h)
{
	// set the initial position for the cube
	set_initial_position();

	// make some randomized colors for the faces
	srand((unsigned)time(NULL));
	for (int i=0; i<6; i++) {
		colors[i] = new double[3];
		for (int j=0; j<3; j++)
			colors[i][j] = (double)(rand()%1000)/1000.0;
	}

	// no transformation update yet, so the last time one happend is zero
	last_time = 0;
}



scene_viewer::~scene_viewer()
{
	// delete the colors we allocated
	for (int i=0; i<6; i++)
		delete[] colors[i];
}


void scene_viewer::draw() 
{
	// check if the current window is valid and setup openGL if neccessary
	if (!valid())
		configure_gl();

	// reset the modelview matrix, set the translation and then our
	// transformation (which is a rotation matrix)
	glLoadIdentity();
	glTranslated(cur_position[0], cur_position[1], cur_position[2]);
	glMultMatrixd(transform);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glBegin(GL_QUADS);
		glColor3dv(colors[0]);
		glNormal3d(-1,0,0);
		glVertex3d(-1, -1, -1);
		glVertex3d(-1, -1, 1);
		glVertex3d(-1, 1, 1);
		glVertex3d(-1, 1, -1);

		glColor3dv(colors[1]);
		glNormal3d(0,0,1);
		glVertex3d(-1, -1, 1);
		glVertex3d(1, -1, 1);
		glVertex3d(1, 1, 1);
		glVertex3d(-1, 1, 1);

		glColor3dv(colors[2]);
		glNormal3d(1,0,0);
		glVertex3d(1, -1, 1);
		glVertex3d(1, -1, -1);
		glVertex3d(1, 1, -1);
		glVertex3d(1, 1, 1);

		glColor3dv(colors[3]);
		glNormal3d(0,0,-1);
		glVertex3d(1, -1, -1);
		glVertex3d(-1, -1, -1);
		glVertex3d(-1, 1, -1);
		glVertex3d(1, 1, -1);

		glColor3dv(colors[4]);
		glNormal3d(0,-1,0);
		glVertex3d(1, -1, -1);
		glVertex3d(1, -1, 1);
		glVertex3d(-1, -1, 1);
		glVertex3d(-1, -1, -1);

		glColor3dv(colors[5]);
		glNormal3d(0,1,0);
		glVertex3d(-1, 1, -1);
		glVertex3d(-1, 1, 1);
		glVertex3d(1, 1, 1);
		glVertex3d(1, 1, -1);
	glEnd();
}


int scene_viewer::handle(int event) 
{
	switch(event) {
		case PUSH:
			// start a callback when the user clicks the gl scene
			// the multiviewer is hooked to this callback
			do_callback();
			return 1;

		// Any other event
		default: 
			return GlWindow::handle(event);
	}
}



void scene_viewer::on_sensor() 
{
	// we got a sensor event. 
	// first get the rotation axis and angle and construct a rotation matrix
	// from these values. also store the translation
	double rotation[4];
	double translation[3];
	double period;

	double delta_transform[16];
	double new_transform[16];

	double cur_time;
	double time_factor = 1;

	// first, get all transformation information
	get_transformation(translation, rotation, &period);


	// to get a smooth animation we measure the elapsed time of the last transformation update.
	// this is done here by calculating how often the device would have been polled
	// in an ideal case. the time in milliseconds between two ideal polls is stored
	// in the variable "period". This will set a time factor which is then a factor
	// for the rotation and the length of the translation vector. it will be 1 if last_time 
	// is zero, which here means that the program was just initialized or the device isn't 
	// moved (see below for the latter case)
	cur_time = fltk::get_time_secs()*1000.0;
	if (last_time != 0)
		time_factor = (cur_time - last_time)/period;

	// the variable rotation[3] stores the rotation angle in degrees. first it gets multiplied
	// by the time factor (see above), then it is converted to radians and then divided by 500
	// to make things smoother. 
	rotation[3] *= (time_factor / 180.0 * 3.1415) / 100.0; 
	// the same smoothing happens to the translation vector
	translation[0] *= time_factor / 1000.0;
	translation[1] *= time_factor / 1000.0;
	translation[2] *= time_factor / 1000.0;

	// to keep the translation independent from the rotation the translation is stored
	// in a different field here
	cur_position[0] += translation[0];
	cur_position[1] += translation[1];
	cur_position[2] += translation[2];


	// calculate the rotation matrix from the axis and the angle
	matrix_rotation(delta_transform, rotation, rotation[3]);

	// now copy the new transformation from left to the old transformation matrix
	// because of subsequent matrix updates on every device update we can easily
	// represent the relative motion
	matrix_multiply(new_transform, delta_transform, transform);

	// copy the matrix to be the actual transformation matrix
	memcpy(transform, new_transform, sizeof(double)*16);

	// as mentioned above things shall be kept smooth so the time difference
	// between this update and the last one is saved. if the device is in its 
	// original position it will not send any data, so the elapsed time between
	// the last call and the call when it is moved again grows which would weighten
	// the first update much too strong. the device sends that it is in the
	// null position before it stops sending update signals. if this is the case
	// we set the last_time variable to zero to indicate that  it is not possible to 
	// calculate the elapsed time in the next update
	if (rotation[3] != 0 || translation[0] != 0 || translation[1] != 0 || translation[2] != 0)
		last_time = cur_time;
	else
		last_time = 0;

	redraw();
}



void scene_viewer::on_keydown() 
{
	// set to initial coords if the FIT button was pressed
	if ((get_keys() & 1<<30))
		set_initial_position();

	redraw();
}



void scene_viewer::configure_gl() 
{
	// this defines the background color to which the frame buffer is set by glClear
	glClearColor(1,1,1,0);
	
	// define the opengl viewport size equal to the fltk-window size given by w() and h()
	glViewport(0,0,w(),h());

	// this command makes all successive matrix manipultations act on the projection transformation stack
	glMatrixMode(GL_PROJECTION);
	// initialize the projective transformation to the identity
	glLoadIdentity();
	// define a viewing pyramid with an opening angle of 45 degrees in the y-direction and 
	// an aspect ration computed from the fltk-window sizes w() and h(). Set the near clipping
	// plane to 0.5 and the far clipping plane to 50
	gluPerspective(45, (float)w()/h(), 0.5, 50);
	// switch back to the modelview transformation stack
	glMatrixMode(GL_MODELVIEW);

	// set the surface material colors and the specular exponent,
	// which is between 0 and 128 and generates sharpest highlights for 128 and no
	// distinguished highlight for 0
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, col_white);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, col_black);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 80); 
	// this mode allows to define the ambient and diffuse color of the surface material
	// via the glColor commands
	glEnable(GL_COLOR_MATERIAL);

	// enable the depth test in order to show only the front most elements of the scene in 
	// each pixel
	glEnable(GL_DEPTH_TEST);
	// cull away the faces whose orientation points away from the observer
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}



void scene_viewer::set_initial_position() 
{
	// Reset the transformation matrix
	matrix_reset(transform);

	// set our actual translation to (0, 0, -5) to have the cube in our view
	cur_position[0] = 0;
	cur_position[1] = 0;
	cur_position[2] = -5;

	redraw();
}




void scene_viewer::matrix_reset(double *matrix)
{
	// Make the identity matrix
	matrix[ 0] = 1;
	matrix[ 1] = 0;
	matrix[ 2] = 0;
	matrix[ 3] = 0;

	matrix[ 4] = 0;
	matrix[ 5] = 1;
	matrix[ 6] = 0;
	matrix[ 7] = 0;

	matrix[ 8] = 0;
	matrix[ 9] = 0;
	matrix[10] = 1;
	matrix[11] = 0;

	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 1;
}



void scene_viewer::matrix_multiply(double *dest, double *m1, double *m2)
{
	// first, reset the destination matrix to zeros
	for (int i=0; i<16; i++)
		dest[i] = 0;

	// multiply the two matrices and store the result in dest
	for (int j=0; j<4; j++)
		for (int i=0; i<4; i++)
			for (int s=0; s<4; s++)
				dest[i+j*4] += m1[s*4+i]*m2[s+j*4];
}



void scene_viewer::matrix_rotation(double *dest, double *axis, double angle)
{
	double axisproduct[9], coslayer[9], sinlayer[9];
	double kronecker;

	// first calculate the outer vector product from the axis
	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
			axisproduct[i+j*3] = axis[i]*axis[j];

	// now calculate (identity_matrix - axisproduct)*cos(angle) to represent a
	// direction in the rotation layer
	for (int i=0; i<9; i++) {
		// calculate the values of the identity matrix at a given point
		// by using the kronecker function
		if (!(i%4))
			kronecker = 1;
		else
			kronecker = 0;

		coslayer[i] = (kronecker - axisproduct[i]) * cos(angle);
	}

	// calculate the cross product matrix * sub(angle) to represent the
	// orthogonal direction to the normal vector
	for (int i=0; i<9; i++)
			sinlayer[i] = 0;
	sinlayer[1] =  axis[2];
	sinlayer[2] = -axis[1];
	sinlayer[3] = -axis[2];
	sinlayer[5] =  axis[0];
	sinlayer[6] =  axis[1];
	sinlayer[7] = -axis[0];
	for (int i=0; i<9; i++)
		sinlayer[i] *= sin(angle);

	// finally add all parts (the axis part for the rotational invariant points
	// along the rotation axis and the both directions in the rotation layer
	matrix_reset(dest);
	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++)
			dest[i+j*4] = axisproduct[i+j*3] + coslayer[i+j*3] + sinlayer[i+j*3];	
}
