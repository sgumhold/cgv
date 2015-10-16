#ifndef __GLSU_H
#define __GLSU_H

/*! \file glsu.h 
   \brief Header of the glsu library.
   
   You only need to include this header to use the glsu library. This is release 2.0 of the 
   OpenGL Stereo Utility library glsu and is Copyright (C) by Stefan Gumhold.

   Changes with respect to release 1.0:
	- included free observer mode
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


/*******************************************************************************************/
/**** Enumeration definitions                                                       ********/
/*******************************************************************************************/

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


/*******************************************************************************************/
/**** Configuration of OpenGL settings necessary to support different stereo approaches ****/
/*******************************************************************************************/


/** configures OpenGL for stereo rendering of left or right eye with given
    stereo mode and in case of anaglyph stereo also for the given anaglyph 
	configuration.

	Stereo mode is turned off by choosing GLSU_CENTER as eye. Stereo rendering
	should be used as in the callback based render process implementation
	in glsuStereoRenderProcess().
*/
void APIENTRY glsuConfigureStereo(enum GlsuEye eye, enum GlsuStereoMode stereoMode, 
								  enum GlsuAnaglyphConfiguration ac);


/*******************************************************************************************/
/*** Simple Stereo API using view angle in y-direction and aspect ratio to define frusti ***/
/*******************************************************************************************/


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

/// same as \c glsuStereoPerspective() but without the translation
void APIENTRY glsuStereoFrustum(enum GlsuEye eye, GLdouble eyeSeparation, 
								GLdouble fovy, GLdouble aspect, 
						        GLdouble zZeroParallax,
						        GLdouble zNear, GLdouble zFar);

/// perform only the translation of the eye
void APIENTRY glsuStereoTranslate(enum GlsuEye eye, GLdouble eyeSeparation,
								  GLdouble fovy, GLdouble aspect, 
								  GLdouble zZeroParallax);

/// perform only the shearing of a monoscopic frustum
void APIENTRY glsuStereoShear(enum GlsuEye eye, GLdouble eyeSeparation,
							  GLdouble fovy, GLdouble aspect, GLdouble zZeroParallax);

/// perform shearing and translation of the eye
void APIENTRY glsuStereoUpdatePerspective(enum GlsuEye eye, GLdouble eyeSeparation, 
	                                      GLdouble fovy, GLdouble aspect, GLdouble zZeroParallax);


/*******************************************************************************************/
/*** Simple Stereo API using screen extend to define frusti                              ***/
/*******************************************************************************************/


/// screen based version of \c glsuStereoRenderProcess()
void APIENTRY glsuStereoRenderProcessScreen(GLdouble eyeSeparation, 
							                GLdouble screenWidth, GLdouble screenHeight, 
							                GLdouble zZeroParallax,
							                GLdouble zNear, GLdouble zFar,
							                void (*drawSceneCallback)(void*), void* userData, 
								            enum GlsuStereoMode mode, enum GlsuAnaglyphConfiguration ac);

/// screen based version of \c glsuStereoPerspective()
void APIENTRY glsuStereoPerspectiveScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
										  GLdouble screenWidth, GLdouble screenHeight, 
										  GLdouble zZeroParallax,
						                  GLdouble zNear, GLdouble zFar);

/// same as \c glsuStereoPerspectiveScreen() but without the translation
void APIENTRY glsuStereoFrustumScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
									  GLdouble screenWidth, GLdouble screenHeight, 
						              GLdouble zZeroParallax,
						              GLdouble zNear, GLdouble zFar);

/// screen based version to perform only the translation of the eye
void APIENTRY glsuStereoTranslateScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
										GLdouble screenWidth);

/// screen based version to perform only the shearing of a monoscopic frustum
void APIENTRY glsuStereoShearScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
									GLdouble screenWidth, GLdouble zZeroParallax);

/// screen based version of shearing and translation of the eye
void APIENTRY glsuStereoUpdatePerspectiveScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
	                                GLdouble screenWidth, GLdouble zZeroParallax);


/*******************************************************************************************/
/*** Free Observer Stereo API uses observer location and direction from left to right eye **/
/*******************************************************************************************/


/// free observer based version of \c glsuStereoRenderProcess()
void APIENTRY glsuStereoRenderProcessFreeObserver(GLdouble eyeSeparation, 
							                      GLdouble screenWidth, GLdouble screenHeight, 
										          GLdouble observerLocation[3], GLdouble eyeSeparationDirection[3],
										          GLdouble zNear, GLdouble zFar, int synchClippingPlanes,
							                      void (*drawSceneCallback)(void*), void* userData, 
								                  enum GlsuStereoMode mode, enum GlsuAnaglyphConfiguration ac);


/// compute the location of the given eye in screen coordinates
GLdouble APIENTRY glsuComputeEyeLocation(enum GlsuEye eye, GLdouble eyeSeparation, GLdouble screenWidth,
							         GLdouble observerLocation[3], GLdouble eyeSeparationDirection[3],
									 double eyeLocation[3]);


/// set stereo perspective for free observer using \c glsuStereoFrustumFreeObserver() and \c glsuStereoTranslateFreeObserver.
void APIENTRY glsuStereoPerspectiveFreeObserver(enum GlsuEye eye, GLdouble eyeSeparation, 
										        GLdouble screenWidth, GLdouble screenHeight, 
										        GLdouble observerLocation[3], GLdouble eyeSeparationDirection[3],
										        GLdouble zNear, GLdouble zFar, int synchClippingPlanes);

/** set view frustum for given eye and eye seperation of an observer located in a coordinate system where the center
    of the screen is the origin and x- and y-directions are synchronized with the rightwards and upwards direction 
	of the screen (z-directions points from screen towards observer -> typically z-coordinates are positive).
	\c eyeSeparationDirection points from left eye to right eye and is automatically normalized if it was not 
	before. If \c synchClippingPlanes is true, the clipping planes for left and right eye are chosen such that
	they correspond to the same plane in space. For this the clipping planes are set relative to the eye closer to
	to the screen. */
void APIENTRY glsuStereoFrustumFreeObserver(enum GlsuEye eye, GLdouble eyeSeparation, 
									        GLdouble screenWidth, GLdouble screenHeight, 
									        GLdouble observerLocation[3], GLdouble eyeSeparationDirection[3],
						                    GLdouble zNear, GLdouble zFar, int synchClippingPlanes);

/// screen based version to perform only the translation of the eye
void APIENTRY glsuStereoTranslateFreeObserver(enum GlsuEye eye, GLdouble eyeSeparation, 
										      GLdouble screenWidth, GLdouble screenHeight, 
										      GLdouble observerLocation[3], GLdouble eyeSeparationDirection[3]);


#ifdef __cplusplus
};
#endif

#endif