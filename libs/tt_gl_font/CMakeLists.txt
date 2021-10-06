
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(tt_gl_font SOURCES ${SOURCES}
        DEPENDENCIES cgv_utils cgv_type cgv_base cgv_media cgv_render cgv_gl)
