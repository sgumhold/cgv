
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(cgv_reflect_types SOURCES ${SOURCES}
        DEPENDENCIES cgv_utils cgv_type cgv_reflect cgv_media cgv_math cgv_render cgv_gui)
