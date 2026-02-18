
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_add_target(cgv_overlay
	TYPE library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	SHADER_SOURCES ${SHADERS}
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

install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_overlay/glsl)
