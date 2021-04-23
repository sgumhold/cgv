
add_subdirectory(cg_ext)
add_subdirectory(cg_fltk)
add_subdirectory(cg_meta)
add_subdirectory(cmi_io)
add_subdirectory(crg_antialias)
add_subdirectory(crg_depth_of_field)
add_subdirectory(crg_grid)
add_subdirectory(crg_light)
add_subdirectory(crg_stereo_view)
add_subdirectory(examples)

if (WIN32)
    # FIXME these plugins can only be compiled under Windows for now
    add_subdirectory(cmv_avi)
    add_subdirectory(co_web)
endif ()
