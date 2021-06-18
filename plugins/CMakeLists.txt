
add_subdirectory(cg_ext)
add_subdirectory(cg_icons)
add_subdirectory(cmf_tt_gl_font)
add_subdirectory(cg_fltk)
add_subdirectory(cg_meta)
add_subdirectory(cmi_io)
add_subdirectory(crg_antialias)
add_subdirectory(crg_depth_of_field)
add_subdirectory(crg_grid)
add_subdirectory(crg_light)
add_subdirectory(crg_stereo_view)
add_subdirectory(examples)

add_subdirectory(crg_vr_wall)
add_subdirectory(openvr_driver)
add_subdirectory(crg_vr_view)
add_subdirectory(vr_test)

if (WIN32)
    # FIXME these plugins can only be compiled under Windows for now
    add_subdirectory(cmv_avi)
    add_subdirectory(co_web)
endif ()

install(EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
