
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE SHADER_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png")

cgv_prepare_resources(${CMAKE_SOURCE_DIR}
        IMG_SOURCES
        IMG_INCLUDES
        ${IMAGES})

add_library(examples
        ${SOURCES}
        ${IMG_SOURCES}
        ${SHADER_SOURCES}
        ${IMAGES})
target_link_libraries(examples
        delaunay
        plot
        cg_gamepad
        cgv_gl
        cgv_os
        cgv_signal)

add_dependencies(examples
        cgv_viewer

        cg_ext
        cg_fltk
        # cg_icons
        cg_meta
        cmi_io
        crg_antialias
        crg_depth_of_field
        crg_grid
        crg_light
        crg_stereo_view)

if (WIN32)
    add_dependencies(examples
            cmv_avi
            co_web)
endif ()

# TODO activate shader tests again
# cgv_test_shaders(examples ${SHADER_SOURCES})

install(TARGETS examples EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
