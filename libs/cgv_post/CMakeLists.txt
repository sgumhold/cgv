
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_add_target(cgv_post
	TYPE library
	SOURCES ${SOURCES}
	SHADER_SOURCES ${SHADERS}
	HEADERS ${HEADERS}

	DEPENDENCIES
		${OPENGL_LIBRARIES} glew cgv_utils cgv_type cgv_data cgv_base cgv_signal
		cgv_media cgv_render cgv_gui cgv_reflect_types cgv_gl
)

if (APPLE)
	target_compile_definitions(cgv_post PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
	target_compile_definitions(cgv_post_static PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
endif ()

install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_post/glsl)
