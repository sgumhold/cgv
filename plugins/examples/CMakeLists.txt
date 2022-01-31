
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png" "*.bmp")

cgv_prepare_resources(${CMAKE_SOURCE_DIR} IMG_SOURCES ${IMAGES})

shader_test("${CGV_DIR}"
        ST_FILES
        ST_INCLUDES
        ST_INSTALL_DIR
        ${SHADERS})

set(ALL_SOURCES ${SOURCES} ${IMG_SOURCES} ${ST_FILES} ${SHADERS} ${IMAGES})

add_library(examples ${ALL_SOURCES})
target_link_libraries(examples
        delaunay
        plot
        cg_gamepad
        cgv_proc
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
        cgv_proc
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

set_plugin_execution_params(examples "plugin:cg_fltk \
\"type(shader_config):shader_path='${CGV_DIR}/plugins/examples;${CGV_DIR}/libs/cgv_gl/glsl;${CGV_DIR}/libs/cgv_proc/glsl'\" \
plugin:cg_ext \
plugin:cg_icons \
plugin:crg_stereo_view \
plugin:crg_antialias \
plugin:crg_depth_of_field \
plugin:crg_light \
plugin:cg_meta \
plugin:cmi_io \
plugin:cmv_avi \
plugin:crg_grid \
plugin:co_web \
plugin:cmf_tt_gl_font \
plugin:examples \
gui:\"${CGV_DIR}/plugins/examples/examples.gui\" \
config:\"${CGV_DIR}/plugins/examples/config.def\"\
")

install(TARGETS examples EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(FILES config.def examples.gui DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/examples)
install(DIRECTORY . DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/examples FILES_MATCHING PATTERN "*.gl*")

if (MSVC)
    add_executable(examples_static ${ALL_SOURCES})
    target_compile_definitions(examples_static PRIVATE CGV_FORCE_STATIC)
    target_link_libraries(examples_static PRIVATE
            cgv_viewer_main

            glew_static
            delaunay_static
            plot_static
            cg_gamepad_static
            cgv_gl_static
            cgv_os_static
            cgv_signal_static
            cgv_proc_static
            cg_fltk_static
            crg_stereo_view_static

            #        cg_ext_static
            #        cg_icons_static
            #        cg_meta_static
            #        cmi_io_static
            #        cmf_tt_gl_font_static
            #        crg_antialias_static
            #        crg_depth_of_field_static
            #        crg_grid_static
            #        crg_light_static
            )
    target_link_options(examples_static PRIVATE /WHOLEARCHIVE:cg_fltk_static.lib /WHOLEARCHIVE:crg_stereo_view_static.lib)
endif ()
