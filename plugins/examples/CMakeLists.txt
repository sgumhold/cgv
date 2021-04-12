
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Shader files
file(GLOB_RECURSE SHADER_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
# Image files
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png")

cgv_prepare_resources(${CMAKE_SOURCE_DIR}
	IMG_SOURCES
	IMG_INCLUDES
	${IMAGES}
)

add_library(examples
	${SOURCES}
	${IMG_SOURCES}
	${SHADER_SOURCES}
	${IMAGES}
)
target_link_libraries(examples
	delaunay
	plot
	cg_gamepad
	cgv_gl
	crg_grid
	crg_stereo_view
	cgv_render
	cgv_gui
	cgv_base
	cgv_type
	cgv_reflect
	cgv_utils
	cgv_signal
	cgv_data
	cgv_media
)
target_include_directories(examples PRIVATE
	../../libs 
	../../libs/delaunay 
)
add_dependencies(examples cgv_viewer)

# cgv_test_shaders(examples ${SHADER_SOURCES})
# set_target_properties(examples PROPERTIES FOLDER "${FOLDER_NAME_APPLICATION_PLUGINS}")

set_target_properties(examples PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "plugin:cg_fltk plugin:crg_stereo_view plugin:crg_antialias plugin:crg_depth_of_field plugin:crg_light plugin:cg_ext plugin:cg_icons plugin:cg_meta plugin:cmi_io plugin:cmv_avi plugin:crg_grid plugin:co_web plugin:examples gui:${CGV_DIR}/plugins/examples/examples.gui config:${CGV_DIR}/plugins/examples/config.def type(shader_config):shader_path='${CGV_DIR}/plugins/examples;${CGV_DIR}/libs/plot/glsl;${CGV_DIR}/libs/cgv_gl/glsl'")
set_target_properties(examples PROPERTIES VS_DEBUGGER_COMMAND $<TARGET_FILE:cgv_viewer>)
