
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx" "*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_add_target(cgv_app
	TYPE library

	SOURCES         ${SOURCES}
	HEADERS         ${HEADERS}
	SHADER_SOURCES  ${SHADERS}

	DEPENDENCIES
		xml glew cgv_type cgv_data cgv_base cgv_signal cgv_math cgv_media cgv_render cgv_gui cgv_reflect_types cgv_gl
		cgv_g2d
)

install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_app/glsl)
