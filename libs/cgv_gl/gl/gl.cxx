#include "gl.h"
#include <iostream>

namespace cgv {
	namespace render {
			namespace gl {

bool& ref_initialized()
{
	static bool is_initialized = false;
	return is_initialized;
}

bool is_glew_initialized()
{
	return ref_initialized();
}

bool ensure_glew_initialized()
{
	static bool called_init = false;
	static bool result;
	if (!called_init) {
		GLenum err = glewInit();
		result = err == GLEW_OK;
		if (!result) {
			std::cerr << "GLEW init error: " << 
				glewGetErrorString(err) << std::endl;
		}
		else {
			ref_initialized() = true;
			called_init = true;
		}
	}
	return result;
}

		}
	}
}