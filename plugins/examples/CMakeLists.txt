cmake_minimum_required(VERSION 2.6)
project(examples)

# The CGV framework is needed
find_package(cgv 
	COMPONENTS render gui base type reflect utils signal data media gl
	HINTS $ENV{CGV_DIR}
	)
	
set(PLUGINS
        cg_fltk
        crg_grid
	crg_stereo_view
)
	
cgv_find_package(cgv_viewer)
cgv_find_package(cgv_gl)
cgv_find_package(crg_grid)
cgv_find_package(crg_stereo_view)
find_package(shader_test)

set(HEADERS 
	shape.h
)

set(SOURCES
	shape.cxx
	shader_test.cxx
)

set(SHADER_SOURCES
	shader_test.glvs
	shader_test.glfs
#	shader_test.glpr
)

include_directories(${cgv_INCLUDE_DIRS} ../../libs ${cgv_gl_INCLUDE_DIRS})

cgv_add_module(examples ${SOURCES} ${HEADERS} ${SHADER_SOURCES})
target_link_libraries(examples ${cgv_LIBRARIES})
cgv_test_shaders(examples ${SHADER_SOURCES})
cgv_add_viewer(test_examples ${PLUGINS} examples)
set_target_properties(examples PROPERTIES FOLDER "${FOLDER_NAME_PLUGINS}")

cgv_write_find_file(examples)
