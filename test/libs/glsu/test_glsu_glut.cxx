#include "test_glsu.h"
#include <GL/glut.h>
#include <iostream>

using namespace std;

static test_glsu test;

static int W, H;

// declare external functions in glut_func.cxx without a header
void resize(int width, int height)
{
	test.set_aspect((GLdouble)width/height);
	glViewport(0,0,width,height);
	W = width;
	H = height;
}

void display()
{
	test.draw();
	glutSwapBuffers();
}

void key(unsigned char key, int x, int y)
{
	bool res = false;
	switch (key) {
	case ' '  : res = test.key_event(Key_SPACE); break;
	case 27   : res = test.key_event(Key_ESCAPE); break;
	case 13 : res = test.key_event(Key_ENTER); break;
	}
	if (res)
		glutPostRedisplay();
}

void motion(int xi, int yi)
{
	bool res = false;
	float x = (float)(xi - W/2)/(W/2);
	float y = (float)(H/2 - yi)/(H/2);
	if (glutGetModifiers() & GLUT_ACTIVE_SHIFT)
		res = test.mouse_event(x, y, 0);
	else if (glutGetModifiers() & GLUT_ACTIVE_ALT)
		res = test.mouse_event(x, y, 1);
	else if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
		res = test.mouse_event(x, y, 2);
	if (res)
		glutPostRedisplay();
}

void special_key(int key, int x, int y)
{
	bool res = false;
	switch (key) {
	case GLUT_KEY_F4 : res = test.key_event(Key_F4); break;
	case GLUT_KEY_F5 : res = test.key_event(Key_F5); break;
	case GLUT_KEY_F6 : res = test.key_event(Key_F6); break;
	case GLUT_KEY_F7 : res = test.key_event(Key_F7); break;
	case GLUT_KEY_F10 : res = test.key_event(Key_F10); break;
	}
	if (res)
		glutPostRedisplay();
}

void timer(int data)
{
	if (test.step_animation())
		glutPostRedisplay();
	glutTimerFunc(20, timer, 0);
}


int main(int argc, char *argv[])
{
	test.show_help();
	// init glut
    glutInitWindowSize(1024,768);
    glutInitWindowPosition(100,20);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STEREO);
	// create window
	glutCreateWindow("test_glsu_glut");
	// set callback handlers
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
	glutPassiveMotionFunc(motion);
    glutSpecialFunc(special_key);
	glutTimerFunc(20, timer, 0);
	// application specific initialization that also initializes opengl
	test.init();
	// start main loop
    glutMainLoop();
    return 0;
}
