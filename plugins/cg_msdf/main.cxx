#if defined(CGV_GUI_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#ifdef MSDF_EXPORTS
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>

CGV_API int dummy_export_msdf = 5;
