
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(vr_lab
	TYPE plugin
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES 
		cgv_base cgv_gl cgv_proc cg_nui cgv_viewer cg_fltk crg_vr_view

	LINKTIME_PLUGIN_DEPENDENCIES crg_vr_view  # <-- we're directly using exported symbols from the crg_vr_view plugin
	                                          # TODO: investigate whether we should instead also link against crg_vr_view_lib
	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)

install(TARGETS vr_lab EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
