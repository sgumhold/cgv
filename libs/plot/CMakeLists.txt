
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

add_library(plot ${SOURCES} ${SHADERS})
target_include_directories(plot PUBLIC ${CGV_DIR}/libs)
target_link_libraries(plot
	cgv_gl
	cgv_utils
	cgv_type
	cgv_reflect
	cgv_data
	cgv_base
	cgv_media
	cgv_os
	cgv_gui
	cgv_render
)
target_compile_definitions(plot PRIVATE PLOT_EXPORTS)
