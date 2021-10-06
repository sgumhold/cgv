
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(cgv_render CORE_LIB SOURCES ${SOURCES}
        DEPENDENCIES cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_signal cgv_math cgv_media cgv_ppp)
