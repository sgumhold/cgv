
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png" "*.bmp")

cgv_prepare_resources(${CMAKE_SOURCE_DIR} IMG_SOURCES ${IMAGES})

shader_test("${CGV_DIR}"
        ST_FILES
        ST_INCLUDES
        ST_INSTALL_DIR
        ${SHADERS})

add_library(examples
        ${SOURCES}
        ${IMG_SOURCES}
        ${ST_FILES}
        ${SHADERS}
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
        cg_icons
        cg_meta
        cmi_io
        cmf_tt_gl_font
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

set_ide_params(examples "plugin:cg_fltk plugin:cg_ext plugin:cg_icons plugin:crg_stereo_view plugin:crg_antialias plugin:crg_depth_of_field plugin:crg_light plugin:cg_meta plugin:cmi_io plugin:cmv_avi plugin:crg_grid plugin:co_web plugin:cmf_tt_gl_font plugin:examples")

install(TARGETS examples EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
