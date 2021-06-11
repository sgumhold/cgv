
# TODO this fixes this error:
#   The imported target "font_test" references the file ".../cgv-install/lib/cgv/vr_test.lib" but this file does not exist.
#   it is unclear whether this causes any other unintended side effects
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(font_test font_test.cxx)
target_link_libraries(font_test
        cgv_utils cgv_type cgv_reflect cgv_data cgv_signal cgv_base 
	cgv_media cgv_gui cgv_render cgv_os tt_gl_font cgv_gl glew)
add_dependencies(font_test
        cgv_viewer cg_fltk cg_icons cmi_io cg_ext crg_stereo_view cmf_tt_gl_font)

install(TARGETS font_test EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
