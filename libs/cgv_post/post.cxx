/*! include the shader files from cgv_post in static builds */

#ifdef REGISTER_SHADER_FILES
#include <cgv/base/register.h>
#include <cgv_post_shader_inc.h>
#endif

#include "lib_begin.h"

CGV_API int dummy_export_post = 5;
