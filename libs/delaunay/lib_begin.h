#ifdef WIN32
#pragma warning(disable:4251)
#endif

#if defined (DELAUNAY_FORCE_STATIC)
#	define DELAUNAY_FORCE_STATIC_LIB
#endif
#ifdef DELAUNAY_EXPORTS
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>

