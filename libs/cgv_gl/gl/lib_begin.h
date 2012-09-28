#if defined(CGV_RENDER_FORCE_STATIC) || defined(CGV_RENDER_GL_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#ifdef CGV_RENDER_GL_EXPORTS
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>
