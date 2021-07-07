
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_core_lib(cgv_gui SOURCES ${SOURCES}
        DEPENDENCIES cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_media cgv_signal)
