
add_subdirectory(cg_gamepad)
add_subdirectory(cgv_gl)
add_subdirectory(cgv_reflect_types)
add_subdirectory(delaunay)
add_subdirectory(gamepad)
add_subdirectory(glsu)
add_subdirectory(plot)

add_custom_target(libs)
add_dependencies(libs
        cg_gamepad
        cgv_gl
        cgv_reflect_types
        delaunay
        gamepad
        glsu
        plot)
