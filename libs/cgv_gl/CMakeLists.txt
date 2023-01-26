
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_add_target(cgv_gl
    TYPE library
    SOURCES ${SOURCES}
    PPP_SOURCES ${PPP_SOURCES}
    SHADER_SOURCES ${SHADERS}
    HEADERS ${HEADERS}
    DEPENDENCIES cgv_utils cgv_type cgv_data cgv_base cgv_signal cgv_math cgv_media cgv_render cgv_gui cgv_reflect_types json glew
)

#cgv_create_lib(cgv_gl SOURCES ${SOURCES} PPP_SOURCES ${PPP_SOURCES} SHADER_SOURCES ${SHADERS}
#        DEPENDENCIES cgv_utils cgv_type cgv_data cgv_base cgv_signal cgv_math cgv_media cgv_render cgv_gui cgv_reflect_types json glew)

target_compile_definitions(cgv_gl PRIVATE CGV_RENDER_GL_EXPORTS)
target_compile_definitions(cgv_gl_static PRIVATE CGV_RENDER_GL_FORCE_STATIC)
#if (WIN32)
#	target_compile_definitions(cgv_gl_static PRIVATE REGISTER_SHADER_FILES)
#endif ()
if (APPLE)
    target_compile_definitions(cgv_gl PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
    target_compile_definitions(cgv_gl_static PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
endif ()

target_link_libraries(cgv_gl PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(cgv_gl_static PUBLIC ${OPENGL_LIBRARIES})

install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_gl/glsl)
