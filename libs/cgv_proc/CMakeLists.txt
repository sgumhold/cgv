
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

cgv_create_lib(cgv_proc SOURCES ${SOURCES} SHADER_SOURCES ${SHADERS}
               DEPENDENCIES cgv_utils cgv_type cgv_reflect cgv_reflect_types cgv_data cgv_base cgv_math cgv_media cgv_render cgv_gl glew)

target_compile_definitions(cgv_proc PRIVATE CGV_PROCEDURAL_EXPORTS)
target_compile_definitions(cgv_proc_static PRIVATE CGV_PROCEDURAL_FORCE_STATIC)
if (APPLE)
    target_compile_definitions(cgv_proc PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
    target_compile_definitions(cgv_proc_static PRIVATE __APPLGLSU__ GL_SILENCE_DEPRECATION)
endif ()

install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/cgv_proc/glsl)
