
add_library(font_test font_test.cxx)
target_link_libraries(font_test
        cgv_utils cgv_type cgv_reflect cgv_data cgv_signal cgv_base 
	cgv_media cgv_gui cgv_render cgv_os tt_gl_font cgv_gl glew)
add_dependencies(font_test
        cgv_viewer cg_fltk cg_icons cmi_io cg_ext crg_stereo_view cmf_tt_gl_font)

install(TARGETS font_test EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
