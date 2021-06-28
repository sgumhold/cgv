
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(tt_gl_font ${SOURCES})
target_compile_definitions(tt_gl_font PRIVATE TT_GL_FONT_EXPORTS)
target_link_libraries(tt_gl_font
        cgv_utils
        cgv_type
        cgv_base
        cgv_media
        cgv_render
        cgv_gl)
target_include_directories(tt_gl_font PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS tt_gl_font EXPORT cgv_libs DESTINATION ${CGV_BIN_DEST})
