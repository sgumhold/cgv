#include "test_glsu.h"
#include <GLFW/glfw3.h>
#include <iostream>

using namespace std;

static test_glsu test;

static int W, H;
bool shift = false;
bool ctrl = false;
bool alt = false;

// declare external functions in glut_func.cxx without a header
void resize(GLFWwindow* window, int width, int height)
{
	test.set_aspect((GLdouble)width / height);
	glViewport(0, 0, width, height);
	W = width;
	H = height;
}

void display()
{
	test.draw();
}

void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	bool state = action == GLFW_RELEASE ? false : true;
	switch (key) {
	case GLFW_KEY_LEFT_SHIFT:
	case GLFW_KEY_RIGHT_SHIFT:
		shift = state; return;
	case GLFW_KEY_LEFT_CONTROL:
	case GLFW_KEY_RIGHT_CONTROL:
		ctrl = state; return;
	case GLFW_KEY_LEFT_ALT:
	case GLFW_KEY_RIGHT_ALT:
		alt = state; return;
	}
	if (action != GLFW_PRESS)
		return;
	bool res = false;

	switch (key) {
	case GLFW_KEY_SPACE: res = test.key_event(Key_SPACE); break;
	case GLFW_KEY_ESCAPE : res = test.key_event(Key_ESCAPE); break;
	case GLFW_KEY_ENTER: res = test.key_event(Key_ENTER); break;
	case GLFW_KEY_F4: res = test.key_event(Key_F4); break;
	case GLFW_KEY_F5: res = test.key_event(Key_F5); break;
	case GLFW_KEY_F6: res = test.key_event(Key_F6); break;
	case GLFW_KEY_F7: res = test.key_event(Key_F7); break;
	case GLFW_KEY_F10: res = test.key_event(Key_F10); break;
	}
	if (res)
		glfwPostEmptyEvent();
}

void motion(GLFWwindow* window, double xd, double yd)
{
	int mode = -1;
	if (shift)
		mode = 0;
	else if (alt)
		mode = 1;
	else if (ctrl)
		mode = 2;
	else
		return;
	bool res = false;
	float x = (float)(xd - W / 2) / (W / 2);
	float y = (float)(H / 2 - yd) / (H / 2);
	res = test.mouse_event(x, y, mode);
	if (res)
		glfwPostEmptyEvent();
}

#include <vector>

struct timer_type
{
	double time;
	int data;
};
void add_timer(unsigned ms, int data);

void timer(int data)
{
	if (test.step_animation())
		glfwPostEmptyEvent();
	add_timer(20, 0);
}

std::vector<timer_type>& ref_timers()
{
	static std::vector<timer_type> timers;
	return timers;
}

void add_timer(unsigned ms, int data)
{
	double new_time = glfwGetTime() + 0.001 * ms;
	for (auto& tr : ref_timers())
		if (tr.data == data) {
			tr.time = new_time;
			return;
		}
	timer_type tr;
	tr.time = new_time;
	tr.data = data;
	ref_timers().push_back(tr);
}

void check_timers(double t)
{
	for (auto& tr : ref_timers())
		if (tr.time <= t)
			timer(tr.data);
}

double next_timer()
{
	if (ref_timers().empty())
		return 0.0;
	double min_t = ref_timers().front().time;
	for (const auto& tr : ref_timers())
		if (tr.time < min_t)
			min_t = tr.time;
	return min_t;
}


int main(int argc, char* argv[])
{
	GLFWwindow* window;
	if (!glfwInit())
		exit(EXIT_FAILURE);
	add_timer(20, 0);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
//	glfwWindowHint(GLFW_STEREO, 1);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	window = glfwCreateWindow(1024, 768, "test_glsu_glfw", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetWindowAspectRatio(window, 4, 3);

	glfwSetFramebufferSizeCallback(window, resize);
	glfwSetKeyCallback(window, key);
	glfwSetCursorPosCallback(window, motion);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	resize(window, width, height);

	
	test.show_help();
	// application specific initialization that also initializes opengl
	test.init();

	glfwSetTime(0.0);
	double t_old = 0.0;
	/* Main loop */
	for (;;)
	{
		/* Timing */
		double t = glfwGetTime();
		check_timers(t);
		double dt = t - t_old;
		t_old = t;
		/* Draw one frame */
		display();
		/* Swap buffers */
		glfwSwapBuffers(window);

		if (ref_timers().empty())
			glfwWaitEvents();
		else {
			double ndt = next_timer() - glfwGetTime();
			if (ndt > 0)
				glfwWaitEventsTimeout(dt);
		}
		glfwPollEvents();

		/* Check if we are still running */
		if (glfwWindowShouldClose(window))
			break;
	}

	glfwTerminate();
	exit(EXIT_SUCCESS);
	return 0;
}
