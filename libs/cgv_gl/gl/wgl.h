#pragma once

/*! file use this header to include opengl, glu, glew and wglew. */
#ifdef WIN32
#include <GL/glew.h>
#include <GL/wglew.h>
#else
#include <GL/glew.h>
#include <GL/glxew.h>
#endif