
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_add_target(holo_disp
	TYPE library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	SHADER_SOURCES ${SHADERS}
	DEPENDENCIES
		cgv_utils cgv_type cgv_data cgv_base cgv_signal cgv_math cgv_media cgv_render cgv_gl
)

if (APPLE)
	target_compile_definitions(holo_disp PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
	target_compile_definitions(holo_disp PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
endif ()

install(DIRECTORY ./glsl DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_gl/glsl)
