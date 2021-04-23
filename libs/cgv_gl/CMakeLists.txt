
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

ppp_compile("${CGV_DIR}"
        PPP_FILES
        PPP_INCLUDES
        PPP_INSTALL_DIR
        gl/gl_context_switch.ph)

add_library(cgv_gl ${SOURCES})
target_compile_definitions(cgv_gl PRIVATE CGV_RENDER_GL_EXPORTS CGV_GL_EXPORTS)
target_link_libraries(cgv_gl
        cgv_utils
        cgv_type
        cgv_data
        cgv_base
        cgv_signal
        cgv_math
        cgv_media
        cgv_render
        cgv_gui
        cgv_reflect_types
        json
        glew
        ${OPENGL_LIBRARIES})

target_include_directories(cgv_gl PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS cgv_gl EXPORT cgv_libs DESTINATION ${CGV_LIB_DEST})
install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_gl/glsl)
