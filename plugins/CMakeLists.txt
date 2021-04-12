
add_subdirectory(cg_fltk)
add_subdirectory(crg_grid)
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
