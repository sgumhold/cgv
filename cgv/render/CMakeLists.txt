
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_render ${SOURCES})
target_compile_definitions(cgv_render PRIVATE CGV_RENDER_EXPORTS)
target_include_directories(cgv_render PUBLIC
	${CGV_DIR}
)
target_link_libraries(cgv_render PUBLIC
	cgv_utils
	cgv_type
	cgv_reflect
	cgv_data
	cgv_base
	cgv_signal
	cgv_math
	cgv_media
	cgv_ppp
)
