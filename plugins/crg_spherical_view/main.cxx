#include <cgv/base/register.h>

#include "spherical_view_interactor.h"

#ifdef _USRDLL
#	define CGV_EXPORTS
#else
#	define CGV_FORCE_STATIC_LIB
#endif
#include <cgv/config/lib_begin.h>

extern CGV_API cgv::base::object_registration_1<spherical_view_interactor,const char*> spherical_view_interactor_instance("trackball");

