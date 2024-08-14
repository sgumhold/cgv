
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx" "*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_add_target(cgv_proc
	TYPE library

	SOURCES         ${SOURCES}
	HEADERS         ${HEADERS}
	SHADER_SOURCES  ${SHADERS}

	DEPENDENCIES
		glew cgv_utils cgv_type cgv_reflect cgv_reflect_types cgv_data cgv_base cgv_math cgv_media cgv_render
		cgv_gl

	OVERRIDE_SHARED_EXPORT_DEFINE
		CGV_PROCEDURAL_EXPORTS

	OVERRIDE_FORCE_STATIC_DEFINE
		CGV_PROCEDURAL_FORCE_STATIC
)

install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_proc/glsl)
