
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_create_lib(plot SOURCES ${SOURCES} SHADER_SOURCES ${SHADERS}
        DEPENDENCIES cgv_gl cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_media cgv_os cgv_gui cgv_render tt_gl_font)

#add_library(plot ${SOURCES} ${SHADERS})
#target_compile_definitions(plot PRIVATE PLOT_EXPORTS)
#target_link_libraries(plot
#        cgv_gl
#        cgv_utils
#        cgv_type
#        cgv_reflect
#        cgv_data
#        cgv_base
#        cgv_media
#        cgv_os
#        cgv_gui
#        cgv_render
#        tt_gl_font)
#target_include_directories(plot PUBLIC
#        $<BUILD_INTERFACE:${CGV_DIR}/libs>
#        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
#
#install(TARGETS plot EXPORT cgv_libs DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/plot/glsl)
