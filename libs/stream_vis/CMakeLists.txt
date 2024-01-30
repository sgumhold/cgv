
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(stream_vis
	TYPE library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES
		glew cgv_utils cgv_type cgv_reflect cgv_data cgv_signal cgv_base
		cgv_media cgv_gui cgv_render cgv_os cgv_gl cgv_app plot
)

# Disable warnings we can't do anything about since they're being emitted erroneously because of a GCC bug 
if (NOT MSVC)
	target_link_options(stream_vis PRIVATE -Wno-stringop-overflow)
	target_link_options(stream_vis_static PRIVATE -Wno-stringop-overflow)
endif()
