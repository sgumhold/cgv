#if defined(CGV_GUI_FORCE_STATIC) || defined(CGV_GUI_G2D_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#if defined(CGV_GUI_G2D_EXPORTS) || defined(CG_G2D_EXPORTS)
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>
