
cgv_add_target(font_test
	TYPE plugin
	SOURCES font_test.cxx
	DEPENDENCIES 
		glew cgv_utils cgv_type cgv_reflect cgv_data cgv_signal cgv_base cgv_media cgv_gui cgv_render cgv_os
		cgv_gl cgv_viewer cg_fltk cg_icons cmi_io cg_ext crg_stereo_view tt_gl_font cmf_tt_gl_font 

	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)
install(TARGETS font_test EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
