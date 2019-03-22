#if defined(CGV_GUI_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#if defined(CGV_MEDIA_IMAGE_IO_EXPORTS) || defined(CMI_IO_EXPORTS)
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>
