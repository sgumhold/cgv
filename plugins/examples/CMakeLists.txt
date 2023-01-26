
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png" "*.bmp")
file(GLOB_RECURSE AUDIO RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.wav")

#cgv_prepare_resources(${CMAKE_SOURCE_DIR} IMG_SOURCES ${IMAGES})
#if(CGV_BUILD_WITH_AUDIO)
#    cgv_prepare_resources(${CMAKE_SOURCE_DIR} AUDIO_SOURCES ${AUDIO})
#else()
#    list(REMOVE_ITEM SOURCES "spatial_audio.cxx")
#    set(AUDIO_SOURCES "")
#    set(AUDIO "")
#endif()

#shader_test("${CGV_DIR}"
#        ST_FILES
#        ST_INCLUDES
#        ST_INSTALL_DIR
#        ${SHADERS})

#set(ALL_SOURCES ${SOURCES} ${IMG_SOURCES} ${AUDIO_SOURCES} ${ST_FILES} ${SHADERS} ${IMAGES} ${AUDIO})

cgv_add_target(examples
    TYPE plugin
    SOURCES ${SOURCES}
    PPP_SOURCES ${PPP_SOURCES}
    HEADERS ${HEADERS}
    SHADER_SOURCES ${SHADERS}
    RESOURCES ${IMG_SOURCES}
    AUDIO_RESOURCES ${AUDIO_SOURCES}
    DEPENDENCIES delaunay plot cg_gamepad cgv_proc cgv_gl cgv_os cgv_signal cgv_g2d cgv_gpgpu cgv_app
)
#add_library(examples ${ALL_SOURCES})
target_link_libraries(examples
        delaunay
        plot
        cg_gamepad
        cgv_proc
        cgv_gl
        cgv_os
        cgv_signal
        cgv_g2d
        cgv_gpgpu
        cgv_app
        )

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

if(CGV_BUILD_WITH_AUDIO)
    target_link_libraries(examples cgv_oal)
    add_dependencies(examples cg_audio)
endif()

if(WIN32)
    add_dependencies(examples
            cmv_avi
            co_web)
endif()

if(CGV_BUILD_WITH_AUDIO)
    set_plugin_execution_params(examples
        "plugin:cg_fltk \"type(shader_config):shader_path='${CGV_DIR}/plugins/examples;${CGV_DIR}/libs/cgv_gl/glsl;${CGV_DIR}/libs/cgv_proc/glsl;${CGV_DIR}/libs/cgv_g2d/glsl;${CGV_DIR}/libs/cgv_gpgpu/glsl;${CGV_DIR}/libs/cgv_app/glsl;${CGV_DIR}/libs/plot/glsl'\" plugin:cg_audio plugin:cg_ext plugin:cg_icons plugin:crg_stereo_view plugin:crg_antialias plugin:crg_depth_of_field plugin:crg_light plugin:cg_meta plugin:cmi_io plugin:cmv_avi plugin:crg_grid plugin:co_web plugin:cmf_tt_gl_font plugin:examples gui:\"${CGV_DIR}/plugins/examples/examples.gui\" config:\"${CGV_DIR}/plugins/examples/config.def\"")
else()
    set_plugin_execution_params(examples
        "plugin:cg_fltk \"type(shader_config):shader_path='${CGV_DIR}/plugins/examples;${CGV_DIR}/libs/cgv_gl/glsl;${CGV_DIR}/libs/cgv_proc/glsl;${CGV_DIR}/libs/cgv_g2d/glsl;${CGV_DIR}/libs/cgv_gpgpu/glsl;${CGV_DIR}/libs/cgv_app/glsl;${CGV_DIR}/libs/plot/glsl'\" plugin:cg_ext plugin:cg_icons plugin:crg_stereo_view plugin:crg_antialias plugin:crg_depth_of_field plugin:crg_light plugin:cg_meta plugin:cmi_io plugin:cmv_avi plugin:crg_grid plugin:co_web plugin:cmf_tt_gl_font plugin:examples gui:\"${CGV_DIR}/plugins/examples/examples.gui\" config:\"${CGV_DIR}/plugins/examples/config.def\"")
endif()

install(TARGETS examples EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(FILES config.def examples.gui DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/examples)
install(DIRECTORY . DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/examples FILES_MATCHING PATTERN "*.gl*")

if (MSVC)
    add_executable(examples_static ${ALL_SOURCES})
    target_compile_definitions(examples_static PRIVATE CGV_FORCE_STATIC)
	#if (WIN32)
	#	target_compile_definitions(examples_static PRIVATE REGISTER_SHADER_FILES)
	#endif ()
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
            cgv_g2d_static
            cgv_gpgpu_static
            cgv_app_static)
    if(CGV_BUILD_WITH_AUDIO)
        target_link_libraries(examples_static PRIVATE cgv_oal_static)
    endif()
    target_link_options(examples_static PRIVATE
            /WHOLEARCHIVE:cg_fltk_static.lib
            /WHOLEARCHIVE:crg_stereo_view_static.lib)
            #/WHOLEARCHIVE:cg_ext_static.lib
            #/WHOLEARCHIVE:cg_icons_static.lib
            #/WHOLEARCHIVE:cg_meta_static.lib
            #/WHOLEARCHIVE:cmi_io_static.lib
            #/WHOLEARCHIVE:cmf_tt_gl_font_static.lib
            #/WHOLEARCHIVE:crg_antialias_static.lib
            #/WHOLEARCHIVE:crg_depth_of_field_static.lib
            #/WHOLEARCHIVE:crg_grid_static.lib
            #/WHOLEARCHIVE:crg_light_static.lib)
endif ()
