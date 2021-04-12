
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cg_fltk ${SOURCES})
target_link_libraries(cg_fltk
	fltk
	cgv_gl
	cgv_gui
	cgv_render
	cgv_media
	cgv_utils
	cgv_os
	cgv_base
	cgv_data
	cgv_type
	cgv_signal
	cgv_reflect
)
target_compile_definitions(cg_fltk PRIVATE CG_FLTK_EXPORTS CGV_GUI_FLTK)

	
# _cgv_set_definitions(cg_fltk 
# 	SHARED FL_SHARED
# 	STATIC ${cgv_gl_STATIC_DEFINITIONS}
# )
# cgv_add_export_definitions(cg_fltk CGV_GUI_FLTK)
