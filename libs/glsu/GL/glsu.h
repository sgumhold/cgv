#ifndef __GLSU_H
#define __GLSU_H

/*! \file glsu.h header of the glsu library
    \brief include this header to use the glsl library.
	
	This is release 1.0 of the OpenGL Stereo Utility library glsu and is Copyright (C) by Stefan Gumhold.
*/

#if defined(_WINDOWS) || defined(WIN32) || defined(WIN64)
#	include <windows.h>
#	include <gl/gl.h>
#else
#	ifndef APIENTRY
#		define APIENTRY
#	endif
#	if defined(__APPLE__) && !defined (VMDMESA) 
#		include <OpenGL/gl.h>
#	else
#		include <GL/gl.h>
#	endif
#endif

/// different stereo modes
enum GlsuStereoMode 
{ 
	GLSU_STEREO_MODE_BEGIN,     /// first possible mode
	GLSU_SPLIT_VERTICALLY = 0,  /// side by side stereo
	GLSU_SPLIT_HORIZONTALLY,    /// top by bottom stereo
	GLSU_ANAGLYPH,              /// anaglyph stereo mode, uses AnaglyphConfiguration
	GLSU_QUAD_BUFFER,           /// quad buffer stereo, only supported on FX Quadro cards
	GLSU_STEREO_MODE_END        /// one more than last mode 
};

/// different configurations in anaglyph mode
enum GlsuAnaglyphConfiguration 
{
	GLSU_ANAGLYPH_CONFIGURATION_BEGIN, /// first possible anaglyph configuration
	GLSU_RED_BLUE = 0,                 /// red filter on left eye, blue on right
	GLSU_RED_CYAN,					   /// red filter on left eye, cyan on right
	GLSU_YELLOW_BLUE,                  /// yellow filter on left eye, blue on right
	GLSU_MAGENTA_GREEN,                /// magenta filter on left eye, green on right
	GLSU_BLUE_RED,                     /// blue filter on left eye, red on right
	GLSU_CYAN_RED,                     /// cyan filter on left eye, red on right
	GLSU_BLUE_YELLOW,                  /// blue filter on left eye, yellow on right
	GLSU_GREEN_MAGENTA,                /// green filter on left eye, magenta on right
	GLSU_ANAGLYPH_CONFIGURATION_END    /// one more than last possible anaglyph configuration
};


/// different eye locations
enum GlsuEye
{
	GLSU_EYE_BEGIN, /// first possible eye
	GLSU_LEFT = -1, /// left eye
	GLSU_CENTER,    /// center eye ... corresponds to location of monoscopic rendering
	GLSU_RIGHT,     /// right eye
	GLSU_EYE_END    /// one more than last possible eye location
};

#ifdef __cplusplus
extern "C" {
#endif

/** configure configures OpenGL for stereo rendering of left or right eye with given
    stereo mode and in case of anaglyph stereo also for the given anaglyph 
	configuration.

	Stereo mode is turned off by choosing GLSU_CENTER as eye. Stereo rendering
	should be used as in the callback based render process implementation
	in glsuStereoRenderProcess().
*/
void APIENTRY glsuConfigureStereo(enum GlsuEye eye, enum GlsuStereoMode stereoMode, 
								  enum GlsuAnaglyphConfiguration ac);

/** Uses the function \c glsuStereoPerspective() to do stereoscopic rendering. The callback
    \c drawSceneCallback() is called once for rendering the scene from the left eye
	and another time from the right eye. The lookat transformation
	is supposed to be set in the \c drawSceneCallback() function.
*/
void APIENTRY glsuStereoRenderProcess(GLdouble eyeSeparation, 
									  GLdouble fovy, GLdouble aspect, 
							          GLdouble zZeroParallax,
							          GLdouble zNear, GLdouble zFar,
							          void (*drawSceneCallback)(void*), void* userData, 
								      enum GlsuStereoMode mode, enum GlsuAnaglyphConfiguration ac);

/// screen based version of \c glsuStereoRenderProcess()
void APIENTRY glsuStereoRenderProcessScreen(GLdouble eyeSeparation, 
							                GLdouble screenWidth, GLdouble screenHeight, 
							                GLdouble zZeroParallax,
							                GLdouble zNear, GLdouble zFar,
							                void (*drawSceneCallback)(void*), void* userData, 
								            enum GlsuStereoMode mode, enum GlsuAnaglyphConfiguration ac);

/** uses \c glFrustum() to multiply a projection matrix to the top of the current 
    matrix stack that enables the frustum of a stereo eye pair including the
	translation from the center to the left or right eye. By performing the 
	translation of the frustum on the projection stack, definition of lights
	local to the observer can be done as before. Also the remainder of the
	rendering process can simlpy use \c gluLookAt() to position the observer
	in the scene. The parameters of \c glsuStereoPerspective() extends the parameters
	to \c gluPerspective() by
*/
void APIENTRY glsuStereoPerspective(enum GlsuEye eye, GLdouble eyeSeparation, 
									GLdouble fovy, GLdouble aspect, 
						            GLdouble zZeroParallax,
						            GLdouble zNear, GLdouble zFar);


/// screen based version of \c glsuStereoPerspective()
void APIENTRY glsuStereoPerspectiveScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
										  GLdouble screenWidth, GLdouble screenHeight, 
										  GLdouble zZeroParallax,
						                  GLdouble zNear, GLdouble zFar);

/// same as \c glsuStereoPerspective() but without the translation
void APIENTRY glsuStereoFrustum(enum GlsuEye eye, GLdouble eyeSeparation, 
								GLdouble fovy, GLdouble aspect, 
						        GLdouble zZeroParallax,
						        GLdouble zNear, GLdouble zFar);

/// same as \c glsuStereoPerspectiveScreen() but without the translation
void APIENTRY glsuStereoFrustumScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
									  GLdouble screenWidth, GLdouble screenHeight, 
						              GLdouble zZeroParallax,
						              GLdouble zNear, GLdouble zFar);

/// perform only the translation of the eye
void APIENTRY glsuStereoTranslate(enum GlsuEye eye, GLdouble eyeSeparation,
								  GLdouble fovy, GLdouble aspect, 
								  GLdouble zZeroParallax);

/// screen based version to perform only the translation of the eye
void APIENTRY glsuStereoTranslateScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
										GLdouble screenWidth);

/// perform only the shearing of a monoscopic frustum
void APIENTRY glsuStereoShear(enum GlsuEye eye, GLdouble eyeSeparation,
							  GLdouble fovy, GLdouble aspect, GLdouble zZeroParallax);

/// screen based version to perform only the shearing of a monoscopic frustum
void APIENTRY glsuStereoShearScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
									GLdouble screenWidth, GLdouble zZeroParallax);

/// perform shearing and translation of the eye
void APIENTRY glsuStereoUpdatePerspective(enum GlsuEye eye, GLdouble eyeSeparation, 
	                                      GLdouble fovy, GLdouble aspect, GLdouble zZeroParallax);

/// screen based version of shearing and translation of the eye
void APIENTRY glsuStereoUpdatePerspectiveScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
	                                GLdouble screenWidth, GLdouble zZeroParallax);


#ifdef __cplusplus
};
#endif

#endif