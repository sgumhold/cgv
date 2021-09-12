
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(cg_vr SOURCES ${SOURCES}
        DEPENDENCIES cgv_utils cgv_type cgv_reflect cgv_reflect_types cgv_data cgv_signal cgv_os cgv_base cgv_math cgv_media cgv_gui vr)
