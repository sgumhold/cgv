#include <cgv/base/register.h>

#include "planar_view_interactor.h"

#if defined(CGV_GUI_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#ifdef CGV_RENDER_PLANAR_VIEW_EXPORTS
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>

/// register a newly created cube with the name "cube1" as constructor argument
extern CGV_API cgv::base::object_registration_1<planar_view_interactor,const char*> obj1("planar_view_interactor");

