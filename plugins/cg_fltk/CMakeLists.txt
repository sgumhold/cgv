cmake_minimum_required(VERSION 2.6)
project(cg_fltk)

#add_subdirectory(/home/jstaib/Entwicklung/CGV-Framework/release)

# The CGV framework is needed
find_package(cgv 
	COMPONENTS gui render media utils os base data type signal reflect
	HINTS $ENV{CGV_DIR}
	)

cgv_find_package(FLTK2)
cgv_find_package(cgv_gl)


# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

include_directories( 
	${FLTK2_INCLUDE_DIRS} 
	${cgv_gl_INCLUDE_DIRS}
	${cgv_INCLUDE_DIRS})

cgv_add_module(cg_fltk ${SOURCES} ${HEADERS})

_cgv_set_definitions(cg_fltk 
	SHARED FL_SHARED
	STATIC ${cgv_gl_STATIC_DEFINITIONS}
)
cgv_add_export_definitions(cg_fltk CGV_GUI_FLTK)

target_link_libraries(cg_fltk ${cgv_gl_LIBRARIES} ${cgv_LIBRARIES} ${FLTK2_LIBRARIES})

cgv_write_find_file(cg_fltk)


