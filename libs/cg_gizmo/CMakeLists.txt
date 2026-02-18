
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx" "*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(cg_gizmo
	TYPE library

	SOURCES         ${SOURCES}
	HEADERS         ${HEADERS}
	SHADER_SOURCES  ${SHADERS}

	DEPENDENCIES
		cgv_base
		cgv_data
		cgv_gl
		cgv_gui
		cgv_reflect
		cgv_render
		cgv_signal
		cgv_type
		cgv_utils
)
