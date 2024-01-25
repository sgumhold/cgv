#include <cgv/base/register.h>


#include "grid.h"

#ifdef _USRDLL
#	define CGV_EXPORTS
#endif
#include <cgv/config/lib_begin.h>

/// register a grid factory
CGV_API cgv::base::factory_registration<grid> grid_fac("grid", "menu_text='New/Grid';shortcut='Ctrl-Alt-G'", true, 
														                "menu_text='&View/Grid';shortcut='Ctrl-G'");
