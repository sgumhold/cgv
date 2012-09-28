#include "test_glsu.h"

#include <stdlib.h>
#include <iostream>

using namespace std;

test_glsu::test_glsu(GLdouble _aspect)
{
	Q = 0;
	animate = true;
	angle = 0;

	mono = false;
	stereo_mode = GLSU_ANAGLYPH;
	anaglyph_configuration   = GLSU_RED_CYAN;
	implementation_mode = IM_AUTOMATIC;
	eyeSeparation = 0.1;
	fovy = 25;
	aspect = _aspect;
	zZeroParallax = 15;
	zNear = 0.1;
	zFar = 100;
}

// set a new aspect ration
void test_glsu::set_aspect(GLdouble _aspect)
{
	aspect = _aspect;
}

bool test_glsu::step_animation()
{
	if (animate) {
		angle += 1;
		return true;
	}
	return false;
}

bool test_glsu::key_event(Key key)
{
	switch (key) {
	case Key_SPACE :
		mono = !mono;
		return true;
	case Key_ESCAPE:
		exit(0);
	case Key_F4 :
		if (++(int&)stereo_mode == GLSU_STEREO_MODE_END)
			stereo_mode = GLSU_STEREO_MODE_BEGIN;
		return true;
	case Key_F5 :
		if (++(int&)anaglyph_configuration == GLSU_ANAGLYPH_CONFIGURATION_END)
			anaglyph_configuration = GLSU_ANAGLYPH_CONFIGURATION_BEGIN;
		return true;
	case Key_F6 :
		if (++(int&)implementation_mode == IM_END)
			implementation_mode = IM_BEGIN;
		return true;
	case Key_F10 :
		animate = !animate;
		return true;
	}
	return false;
}


void test_glsu::render()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	if (!Q)
		Q = gluNewQuadric();

	gluLookAt(0,0,15,0,0,0,0,1,0);

	GLfloat position[] = { 2.0f, 1.0f, 5.0f, 1.0f };
	GLfloat position1[] = { 0.0f, 3.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glLightfv(GL_LIGHT1, GL_POSITION, position1);

	glRotated(angle,0,0,1);
	gluCylinder(Q, 2, 1, 2, 200, 100);
	glRotated(angle-90,0,1,0);
	gluCylinder(Q, 1, 0.02, 3, 200, 200);
	glRotated(90,1,0,0);
	glTranslated(0,1,-3);
	gluCylinder(Q, 0.1, 0.1, 6, 200, 200);
	glTranslated(0,-1,3);
	glRotated(90,0,1,0);
	glTranslated(0,1,-3);
	gluCylinder(Q, 0.1, 0.1, 6, 20, 200);
}

void test_glsu::draw_callback(void* this_ptr)
{
	static_cast<test_glsu*>(this_ptr)->render();
}

void test_glsu::draw_from_eye_physical(GlsuEye eye)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glsuStereoFrustum(eye, eyeSeparation, fovy, aspect, zZeroParallax, zNear, zFar);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glsuStereoTranslate(eye, eyeSeparation, fovy, aspect, zZeroParallax);
	
	render();
	glFlush();
}

void test_glsu::draw_from_eye_convenient(GlsuEye eye)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glsuStereoPerspective(eye, eyeSeparation, fovy, aspect, zZeroParallax, zNear, zFar);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	render();
	glFlush();
}

void test_glsu::draw_from_eye(GlsuEye eye)
{
	if (implementation_mode == IM_PHYSICAL) {
		glClearColor(0.5,0.7f,0.5,1);
		draw_from_eye_physical(eye);
	}
	else {
		glClearColor(0.7f,0.5,0.5,1);
		draw_from_eye_convenient(eye);
	}
}

// initialization is called once in the beginning
void test_glsu::init()
{
	// basic config
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

	// define lights
	GLfloat light[] = { 0.7f,0.7f,0.7f,1 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// define material
	GLfloat ambient[] = { 0.1f, 0.02f, 0.1f, 1.0f };
	GLfloat diffuse[] = { 0.7f, 0.6f, 0.5f, 1.0f };
	GLfloat specular[] = { 1,1,1,1 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50);
}

// redraws the window
void test_glsu::draw()
{
	// monoscopic rendering
	if (mono) {
		glClearColor(0.5,0.5,0.5,1);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(fovy, aspect, zNear, zFar);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		render();
	}
	// stereo done through convenience call
	else if (implementation_mode == IM_AUTOMATIC) {
		glClearColor(0.5,0.5,0.7f,1);
		glsuStereoRenderProcess(eyeSeparation, fovy, aspect, zZeroParallax, zNear, zFar, &test_glsu::draw_callback, this, stereo_mode, anaglyph_configuration);
	}
	// stereo done by yourself
	else {
		glsuConfigureStereo(GLSU_LEFT, stereo_mode, anaglyph_configuration);
		draw_from_eye(GLSU_LEFT);
		glsuConfigureStereo(GLSU_RIGHT, stereo_mode, anaglyph_configuration);
		draw_from_eye(GLSU_RIGHT);
		glsuConfigureStereo(GLSU_CENTER, stereo_mode, anaglyph_configuration);
	}			
}

// stream help information to cout
void test_glsu::show_help() const
{
	cout << "Keyboard control:\n  <Space>:toggle mono (gray background)/stereo\n   <F4>:navigate stereo modes\n"
		    "   <F5>:navigate anaglyph configurations\n   <F6>:toggle implementation mode\n"
			"      automatic (blue background) uses gluStereoRenderProcess\n"
			"      physical (green background) performs gluStereoTranslate on model view\n"
			"      convenient (red background) performs gluStereoTranslate on projection\n"
			"   <F10>:toggle animation" << endl;
}