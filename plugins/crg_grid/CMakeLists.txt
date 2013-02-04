cmake_minimum_required(VERSION 2.6)
project(crg_grid)

# The CGV framework is needed
find_package(cgv 
	COMPONENTS render gui base type reflect utils signal data gl
	HINTS $ENV{CGV_DIR}
	)
	
cgv_find_package(cgv_gl)

set(HEADERS 
	grid.h)

set(SOURCES
	grid.cxx
	main.cxx)

include_directories(${cgv_INCLUDE_DIRS} ../../libs ${cgv_gl_INCLUDE_DIRS})

cgv_add_module(crg_grid ${SOURCES} ${HEADERS})
target_link_libraries(crg_grid ${cgv_LIBRARIES})

set_target_properties(crg_grid PROPERTIES FOLDER "${FOLDER_NAME_PLUGINS}")

cgv_write_find_file(crg_grid)

