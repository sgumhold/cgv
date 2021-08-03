
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(cgv_ppp CORE_LIB SOURCES ${SOURCES} DEPENDENCIES cgv_utils cgv_type)
