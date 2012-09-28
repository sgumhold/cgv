#include <GL/glsu.h>

#if defined(__APPLGLSU__) && !defined (VMDMESA) 
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <math.h>

static unsigned int cms[8][2][3] = { 
	{ {1,0,0}, {0,0,1} },
	{ {1,0,0}, {0,1,1} },
	{ {1,1,0}, {0,0,1} },
	{ {1,0,1}, {0,1,0} },

	{ {0,0,1}, {1,0,0} },
	{ {0,1,1}, {1,0,0} },
	{ {0,0,1}, {1,1,0} },
	{ {0,1,0}, {1,0,1} }
};

GLdouble glsuGetScreenWidth(GLdouble fovy, GLdouble aspect, GLdouble zZeroParallax, GLdouble* screenHeightPtr)
{
	GLdouble screenHeight = 2*tan(.8726646262e-2*fovy)*zZeroParallax;
	if (screenHeightPtr)
		*screenHeightPtr = screenHeight;
	return screenHeight*aspect;
}

void APIENTRY glsuConfigureStereo(enum GlsuEye eye, 
								  enum GlsuStereoMode stereo_mode, 
								  enum GlsuAnaglyphConfiguration ac)
{
	static GLint vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);
	switch (eye) {
	case GLSU_LEFT   : 
		switch (stereo_mode) {
		case GLSU_SPLIT_VERTICALLY:
			glViewport(vp[0],vp[1],vp[2]/2,vp[3]); 
			glScissor(vp[0],vp[1],vp[2]/2,vp[3]); 
			glEnable(GL_SCISSOR_TEST);
			break;
		case GLSU_SPLIT_HORIZONTALLY:
			glViewport(vp[0],vp[1],vp[2],vp[3]/2); 
			glScissor(vp[0],vp[1],vp[2],vp[3]/2); 
			glEnable(GL_SCISSOR_TEST);
			break;
		case GLSU_ANAGLYPH:
			glColorMask(cms[ac][0][0],cms[ac][0][1],cms[ac][0][2],1);
			break;
		case GLSU_QUAD_BUFFER :
			glDrawBuffer(GL_BACK_LEFT);
			break;
		}
		break;
	case GLSU_RIGHT  : 
		switch (stereo_mode) {
		case GLSU_SPLIT_VERTICALLY :
			glViewport(vp[0]+vp[2],vp[1],vp[2],vp[3]); 
			glScissor(vp[0]+vp[2],vp[1],vp[2],vp[3]); 
			glEnable(GL_SCISSOR_TEST);
			break;
		case GLSU_SPLIT_HORIZONTALLY :
			glViewport(vp[0],vp[1]+vp[3],vp[2],vp[3]); 
			glScissor(vp[0],vp[1]+vp[3],vp[2],vp[3]); 
			glEnable(GL_SCISSOR_TEST);
			break;
		case GLSU_ANAGLYPH :
			glColorMask(cms[ac][1][0],cms[ac][1][1],cms[ac][1][2],1);
			break;
		case GLSU_QUAD_BUFFER :
			glDrawBuffer(GL_BACK_RIGHT);
			break;
		}
		break;
	case GLSU_CENTER : 
		switch (stereo_mode) {
		case GLSU_SPLIT_VERTICALLY:
			glViewport(vp[0]-vp[2],vp[1],2*vp[2],vp[3]); 
			glScissor(vp[0]-vp[2],vp[1],2*vp[2],vp[3]); 
			glDisable(GL_SCISSOR_TEST);
			break;
		case GLSU_SPLIT_HORIZONTALLY:
			glViewport(vp[0],vp[1]-vp[3],vp[2],2*vp[3]); 
			glScissor(vp[0],vp[1]-vp[3],vp[2],2*vp[3]); 
			glDisable(GL_SCISSOR_TEST);
			break;
		case GLSU_ANAGLYPH:
			glColorMask(1,1,1,1);
			break;
		case GLSU_QUAD_BUFFER :
			glDrawBuffer(GL_BACK);
			break;
		}
		break;
	}
}

/// same as glsuStereoPerspectiveScreen but without the translation
void APIENTRY glsuStereoFrustumScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
							 GLdouble screenWidth, GLdouble screenHeight, 
							 GLdouble zZeroParallax,
							 GLdouble zNear, GLdouble zFar)
{
	double aspect = screenWidth / screenHeight;
	double top    = 0.5*screenHeight*zNear / zZeroParallax;
	double bottom = -top;
	double delta  = 0.5*eyeSeparation*eye*screenWidth*zNear / zZeroParallax;
	double left   = bottom*aspect - delta;
	double right  =    top*aspect - delta;
	glFrustum(left,right,bottom,top,zNear,zFar);
}

/// same as glsuStereoPerspective but without the translation
void APIENTRY glsuStereoFrustum(enum GlsuEye eye, GLdouble eyeSeparation, 
					   GLdouble fovy, GLdouble aspect, 
					   GLdouble zZeroParallax,
					   GLdouble zNear, GLdouble zFar)
{
	GLdouble screenHeight;
	GLdouble screenWidth = glsuGetScreenWidth(fovy, aspect, zZeroParallax, &screenHeight);
	glsuStereoFrustumScreen(eye, eyeSeparation, screenWidth, screenHeight, zZeroParallax, zNear, zFar);
}


/// perform only the translation of the eye
void APIENTRY glsuStereoTranslateScreen(enum GlsuEye eye, GLdouble eyeSeparation, GLdouble screenWidth)
{
	glTranslated(-0.5*eyeSeparation*eye*screenWidth, 0, 0);
}

void APIENTRY glsuStereoTranslate(enum GlsuEye eye, GLdouble eyeSeparation, GLdouble fovy, GLdouble aspect, GLdouble zZeroParallax)
{
	glsuStereoTranslateScreen(eye, eyeSeparation, glsuGetScreenWidth(fovy, aspect, zZeroParallax, 0));
}

void APIENTRY glsuStereoPerspectiveScreen(enum GlsuEye eye, GLdouble eyeSeparation, 
								 GLdouble screenWidth, GLdouble screenHeight, 
								 GLdouble zZeroParallax,
								 GLdouble zNear, GLdouble zFar)
{
	glsuStereoFrustumScreen(eye, eyeSeparation, screenWidth, screenHeight, zZeroParallax, zNear, zFar);
	glsuStereoTranslateScreen(eye, eyeSeparation, screenWidth);
}

void APIENTRY glsuStereoPerspective(enum GlsuEye eye, GLdouble eyeSeparation, 
						   GLdouble fovy, GLdouble aspect, 
						   GLdouble zZeroParallax,
						   GLdouble zNear, GLdouble zFar)
{
	GLdouble screenHeight;
	GLdouble screenWidth  = glsuGetScreenWidth(fovy, aspect, zZeroParallax, &screenHeight);
	glsuStereoPerspectiveScreen(eye, eyeSeparation, screenWidth, screenHeight, zZeroParallax, zNear, zFar);
}



void APIENTRY glsuStereoUpdatePerspectiveScreen(enum GlsuEye eye, GLdouble eyeSeparation, GLdouble screenWidth, GLdouble zZeroParallax)
{
	GLdouble t_x  = -0.5*eyeSeparation*eye*screenWidth;
	GLdouble s_xz = t_x / zZeroParallax;
	GLdouble M[16] = {
		   1, 0, 0, 0,
		   0, 1, 0, 0,
		s_xz, 0, 1, 0,
		 t_x, 0, 0, 1
	};
	glMultMatrixd(M);
}

void APIENTRY glsuStereoUpdatePerspective(enum GlsuEye eye, GLdouble eyeSeparation, GLdouble fovy, GLdouble aspect, GLdouble zZeroParallax)
{
	glsuStereoUpdatePerspectiveScreen(eye, eyeSeparation, glsuGetScreenWidth(fovy, aspect, zZeroParallax, 0), zZeroParallax);
}

void APIENTRY glsuStereoShearScreen(enum GlsuEye eye, GLdouble eyeSeparation, GLdouble screenWidth, GLdouble zZeroParallax)
{
	GLdouble s_xz = -0.5*eyeSeparation*eye*screenWidth / zZeroParallax;
	GLdouble M[16] = {
		   1, 0, 0, 0,
		   0, 1, 0, 0,
		s_xz, 0, 1, 0,
		   0, 0, 0, 1
	};
	glMultMatrixd(M);
}

void APIENTRY glsuStereoShear(enum GlsuEye eye, GLdouble eyeSeparation, GLdouble fovy, GLdouble aspect, GLdouble zZeroParallax)
{
	glsuStereoShearScreen(eye, eyeSeparation, glsuGetScreenWidth(fovy, aspect, zZeroParallax, 0), zZeroParallax);
}


void APIENTRY glsuStereoRenderProcess(GLdouble eyeSeparation, 
						     GLdouble fovy, GLdouble aspect, 
						     GLdouble zZeroParallax,
						     GLdouble zNear, GLdouble zFar,
						     void (*drawSceneCallback)(void*), void* userData, 
							 enum GlsuStereoMode mode, enum GlsuAnaglyphConfiguration ac)
{
	glsuConfigureStereo(GLSU_LEFT, mode, ac);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glsuStereoPerspective(GLSU_LEFT, eyeSeparation, fovy, aspect, zZeroParallax, zNear, zFar);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	drawSceneCallback(userData);
	glFlush();

	glsuConfigureStereo(GLSU_RIGHT, mode, ac);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glsuStereoPerspective(GLSU_RIGHT, eyeSeparation, fovy, aspect, zZeroParallax, zNear, zFar);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	drawSceneCallback(userData);
	glFlush();

	glsuConfigureStereo(GLSU_CENTER, mode, ac);
}

void APIENTRY glsuStereoRenderProcessScreen(GLdouble eyeSeparation, 
								   GLdouble screenWidth, GLdouble screenHeight, 
								   GLdouble zZeroParallax,
								   GLdouble zNear, GLdouble zFar,
								   void (*drawSceneCallback)(void*), void* userData, 
								   enum GlsuStereoMode mode, enum GlsuAnaglyphConfiguration ac)
{
	glsuConfigureStereo(GLSU_LEFT, mode, ac);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glsuStereoPerspectiveScreen(GLSU_LEFT, eyeSeparation, screenWidth, screenHeight, zZeroParallax, zNear, zFar);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	drawSceneCallback(userData);
	glFlush();

	glsuConfigureStereo(GLSU_RIGHT, mode, ac);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glsuStereoPerspectiveScreen(GLSU_RIGHT, eyeSeparation, screenWidth, screenHeight, zZeroParallax, zNear, zFar);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	drawSceneCallback(userData);
	glFlush();

	glsuConfigureStereo(GLSU_CENTER, mode, ac);
}
