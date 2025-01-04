
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_LIST_DIR}" "*.gl*")

cgv_add_target(maptiles_plugin
	TYPE plugin NO_EXECUTABLE
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}
	SHADER_SOURCES ${SHADERS}

	DEPENDENCIES
		cgv_utils cgv_type cgv_reflect cgv_data cgv_signal cgv_base cgv_media cgv_gui cgv_render cgv_os fltk cgv_gl
		glew cgv_viewer cg_fltk cg_ext crg_stereo_view crg_bg cg_meta cmi_io cgv_app maptiles

	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)

install(TARGETS maptiles_plugin EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
