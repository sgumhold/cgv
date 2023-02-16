
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(vr_test
	TYPE plugin
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES 
		glew cgv_utils cgv_type cgv_data cgv_base cgv_math cgv_media cgv_gui cgv_render cgv_gl cg_vr
		vr_emulator cgv_viewer cg_fltk cmi_io cg_ext crg_grid openvr_driver crg_vr_view crg_vr_wall

	LINKTIME_PLUGIN_DEPENDENCIES crg_vr_view  # <-- we're directly using exported symbols from the crg_vr_view plugin
	                                          # TODO: investigate whether we should instead also link against crg_vr_view_lib

	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)

install(TARGETS vr_test EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
