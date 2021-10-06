file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
add_library(cmf_tt_gl_font ${SOURCES})
target_compile_definitions(cmf_tt_gl_font PRIVATE CMF_TT_GL_FONT_EXPORTS)
target_link_libraries(cmf_tt_gl_font
        cgv_utils cgv_type cgv_base cgv_media cgv_render cgv_gl tt_gl_font glew)
target_include_directories(cmf_tt_gl_font PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
install(TARGETS cmf_tt_gl_font EXPORT cgv_libs DESTINATION ${CGV_BIN_DEST})
