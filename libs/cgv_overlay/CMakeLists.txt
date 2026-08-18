
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(cgv_overlay
	TYPE library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES
		cgv_base
		cgv_data
		cgv_g2d
		cgv_gl
		cgv_gui
		cgv_media
		cgv_os
		cgv_reflect
		cgv_render
		cgv_type
		cgv_utils
		cg_msdf
)
