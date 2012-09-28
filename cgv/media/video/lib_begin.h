// begin block to configure defines for the media_video library
//
// Authored by Stefan Gumhold
//
// Please report all bugs and problems to "cgv@inf.tu-dresden.de".

#if defined (CGV_MEDIA_FORCE_STATIC) || defined (CGV_MEDIA_VIDEO_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#ifdef CGV_MEDIA_VIDEO_EXPORTS
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>


