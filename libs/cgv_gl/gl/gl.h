#pragma once

/*! file use this header to include opengl, glu and glew in the right order.
    If you need wglew.h, include <cgv_gl/gl/wgl.h> instead. */

#include <GL/glew.h>

#include "lib_begin.h"

namespace cgv {
	namespace render {
		namespace gl {

/** traits structure that maps a fundamental type to the Type enum. */
template <typename T>
struct gl_traits { static const unsigned type = GL_FALSE; };
template <> 
struct gl_traits<unsigned char> { static const unsigned type = GL_UNSIGNED_BYTE; };
template <> 
struct gl_traits<char> { static const unsigned type = GL_BYTE; };
template <> 
struct gl_traits<unsigned short> { static const unsigned type = GL_UNSIGNED_SHORT; };
template <> 
struct gl_traits<short> { static const unsigned type = GL_SHORT; };
template <> 
struct gl_traits<unsigned int> { static const unsigned type = GL_UNSIGNED_INT; };
template <> 
struct gl_traits<int> { static const unsigned type = GL_INT; };
template <> 
struct gl_traits<float> { static const unsigned type = GL_FLOAT; };
template <> 
struct gl_traits<double> { static const unsigned type = GL_DOUBLE; };


/// initialize glew in the first call to this function and always return whether this was successful
extern CGV_API bool ensure_glew_initialized();
extern CGV_API bool is_glew_initialized();

		}
	}
}

#include <cgv/config/lib_end.h>

