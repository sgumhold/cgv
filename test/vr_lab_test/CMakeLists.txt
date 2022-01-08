file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(vr_lab_test SOURCES ${SOURCES}
			DEPENDENCIES cgv_utils, cgv_type, cgv_data, cgv_base, cgv_math, cgv_media, cgv_gui, cgv_render, 
						 cgv_gl, cg_vr, plot, cgv_viewer, cg_fltk, cmf_tt_gl_font, crg_grid, cg_ext, cmi_io, crg_vr_view, vr_lab,
						 vr_emulator, openvr_driver)

