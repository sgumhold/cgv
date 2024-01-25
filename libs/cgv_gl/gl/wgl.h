#pragma once

/*! file use this header to include opengl, glu, glew and wglew. */
#ifdef WIN32
#define NOMINMAX
#include <GL/glew.h>
#include <GL/wglew.h>
#undef NOMINMAX
#else
#include <GL/glew.h>
#include <GL/glxew.h>
#endif