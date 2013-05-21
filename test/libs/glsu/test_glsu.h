#pragma once

#include <GL/glsu.h>

/** \file test_glsu.h 
    \brief header of class test_glsu that encapsulate common test functionality
*/

#if defined(__APPLGLSU__) && !defined (VMDMESA) 
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

/// selection of possible implementation
enum ImplementationMode  {
	IM_BEGIN,
	IM_AUTOMATIC = 0,
	IM_PHYSICAL,
	IM_CONVENIENT,
	IM_END
};

/// keys that support some action
enum Key {
	Key_SPACE,  /// toggle mono / stereo mode
	Key_ENTER,  /// reset free observer
	Key_ESCAPE, /// exits application
	Key_F4,     /// changes the stereo mode
	Key_F5,     /// changes the anaglyph configuration
	Key_F6,     /// changes implementation mode
	Key_F7,     /// toggles free observer
	Key_F10     /// toggles animation
};

//! the \c test_glsu class encapsulates functionality to test the glsu library
class test_glsu
{
protected:
	// members used during rendering
	GLUquadricObj* Q;
	bool animate;
	GLdouble angle;

	// configuration parameters
	bool mono;
	GlsuStereoMode stereo_mode;
	GlsuAnaglyphConfiguration anaglyph_configuration;
	ImplementationMode implementation_mode;

	// stereo camera parameters
	GLdouble eyeSeparation;
	GLdouble fovy;
	GLdouble aspect;
	GLdouble zZeroParallax;
	GLdouble zNear;
	GLdouble zFar;

	GLdouble screenHeight;
	// free observer parameters
	bool useFreeObserver;
	GLdouble observerLocation[3];
	GLdouble eyeSeparationDirection[3];
	int synchClippingPlanes;
public:
	/// construct from given aspect ratio
	test_glsu(GLdouble _aspect = 4.0/3);
	/// set a new aspect ration
	void set_aspect(GLdouble _aspect);
	/// step animation
	bool step_animation();
	/// handle key event
	bool key_event(Key key);
	///
	bool wheel_event(float delta);
	/// handle mouse event to set location of free observer
	bool mouse_event(float x, float y, int modifier);
	/// pure opengl based rendering of scene independent of current eye
	void render();
	/// callback used for automatic implementation mode
	static void draw_callback(void* this_ptr);
	/// physically based light computation
	void draw_from_eye_physical(GlsuEye eye);
	/// more convenient light computation
	void draw_from_eye_convenient(GlsuEye eye);
	/// dispatched to \c draw_from_eye_physical or \c draw_from_eye_convenient
	void draw_from_eye(GlsuEye eye);
	/// initialization is called once in the beginning
	void init();
	/// draw scene stereoscopically
	void draw();
	/// stream help information to cout
	void show_help() const;
};

