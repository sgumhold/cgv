
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(vr_lab_test
	TYPE plugin
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES 
		plot crg_vr_view vr_lab

	# we're directly using exported symbols from these plugins
	LINKTIME_PLUGIN_DEPENDENCIES crg_vr_view vr_lab

	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)

#add_library(vr_lab_test ${SOURCES})
#target_link_libraries(vr_lab_test vr_lab plot)
#add_dependencies(vr_lab_test cg_fltk cmf_tt_gl_font crg_grid cg_ext cmi_io crg_vr_view vr_lab vr_emulator openvr_driver)

install(TARGETS vr_lab_test EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(FILES config.def DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/vr_lab_test)
