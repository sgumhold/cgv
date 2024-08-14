
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx" "*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_add_target(plot
	TYPE library

	SOURCES         ${SOURCES}
	HEADERS         ${HEADERS}
	SHADER_SOURCES  ${SHADERS}

	DEPENDENCIES
        cgv_gl cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_media cgv_os cgv_gui cgv_render tt_gl_font
)

install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/plot/glsl)
