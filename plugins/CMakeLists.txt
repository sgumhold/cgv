
add_subdirectory(cg_ext)
add_subdirectory(cg_fltk)
add_subdirectory(cg_meta)
add_subdirectory(cmi_io)
add_subdirectory(cmv_avi)
# add_subdirectory(co_web)
add_subdirectory(crg_antialias)
add_subdirectory(crg_depth_of_field)
add_subdirectory(crg_grid)
add_subdirectory(crg_light)
add_subdirectory(crg_stereo_view)

add_custom_target(plugins)
add_dependencies(plugins
        cg_fltk
        crg_grid
        crg_stereo_view)

if (BUILD_EXAMPLE_PLUGIN)
    add_subdirectory(examples)
    add_dependencies(plugins examples)
endif ()
