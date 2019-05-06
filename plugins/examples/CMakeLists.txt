cmake_minimum_required(VERSION 2.6)
project(examples)

# The CGV framework is needed
find_package(cgv 
	COMPONENTS render gui base type reflect utils signal data media
	HINTS $ENV{CGV_DIR}
	)
	
set(PLUGINS
    cg_fltk
    crg_grid
	crg_stereo_view
)
	
cgv_find_package(delaunay)
cgv_find_package(plot)
cgv_find_package(cgv_gl)
cgv_find_package(crg_grid)
cgv_find_package(crg_stereo_view)

find_package(shader_test)
find_package(cgv_viewer)
find_package(res_prep)

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Shader files
file(GLOB_RECURSE SHADER_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
# Image files
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png")

cgv_prepare_resources(${CMAKE_SOURCE_DIR} IMG_SOURCES ${IMAGES})

include_directories(
	${cgv_INCLUDE_DIRS} 
	../../libs 
	../../libs/delaunay 
	${cgv_gl_INCLUDE_DIRS}
)

cgv_add_module(examples ${SOURCES} ${IMG_SOURCES} ${HEADERS} ${SHADER_SOURCES} ${IMAGES})
target_link_libraries(examples ${cgv_LIBRARIES} ${delaunay_LIBRARIES} ${plot_LIBRARIES})
cgv_test_shaders(examples ${SHADER_SOURCES})
set_target_properties(examples PROPERTIES FOLDER "${FOLDER_NAME_APPLICATION_PLUGINS}")

cgv_get_viewer_locations(VIEWER_EXE VIEWER_DEBUG_EXE)
set_target_properties(examples PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "plugin:cg_fltk plugin:crg_stereo_view plugin:crg_antialias plugin:crg_depth_of_field plugin:crg_light plugin:cg_ext plugin:cg_icons plugin:cg_meta plugin:cmi_io plugin:cmv_avi plugin:crg_grid plugin:co_web plugin:examples gui:${CGV_DIR}/plugins/examples/examples.gui config:${CGV_DIR}/plugins/examples/config.def type(shader_config):shader_path='${CGV_DIR}/plugins/examples;${CGV_DIR}/libs/plot/glsl;${CGV_DIR}/libs/cgv_gl/glsl'")
set_target_properties(examples PROPERTIES VS_DEBUGGER_COMMAND $<IF:$<CONFIG:Debug>,${VIEWER_DEBUG_EXE},${VIEWER_EXE}>)

cgv_write_find_file(examples)
