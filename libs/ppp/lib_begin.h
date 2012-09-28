#if defined(CGV_RENDER_FORCE_STATIC) || defined(CGV_MEDIA_TEXT_FORCE_STATIC) || defined(CGV_MEDIA_TEXT_PPP_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#if defined(CGV_MEDIA_TEXT_PPP_EXPORTS) || defined(CGV_RENDER_EXPORTS)
#	define CGV_EXPORTS
#endif


#include <cgv/config/lib_begin.h>
