#if defined(CGV_GUI_FORCE_STATIC) || defined(CGV_GUI_GLFW_IMGUI_FORCE_STATIC)
#	define CGV_FORCE_STATIC_LIB
#endif
#if defined(CGV_GUI_GLFW_IMGUI_EXPORTS) || defined(CG_GLFW_IMGUI_EXPORTS)
#	define CGV_EXPORTS
#endif

#include <cgv/config/lib_begin.h>
