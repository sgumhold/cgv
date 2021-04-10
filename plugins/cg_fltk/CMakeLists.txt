
# The CGV framework is needed
# find_package(cgv 
# 	COMPONENTS gui render media utils os base data type signal reflect
# 	HINTS $ENV{CGV_DIR}
# 	)

# cgv_find_package(FLTK2)
# cgv_find_package(cgv_gl)


# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

add_library(cg_fltk ${SOURCES})
target_link_libraries(cg_fltk
	fltk
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
	cgv_gl
)
# target_include_directories(cg_fltk PUBLIC)

	
# _cgv_set_definitions(cg_fltk 
# 	SHARED FL_SHARED
# 	STATIC ${cgv_gl_STATIC_DEFINITIONS}
# )
# cgv_add_export_definitions(cg_fltk CGV_GUI_FLTK)
