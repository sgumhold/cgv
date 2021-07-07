
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_core_lib(cgv_math SOURCES ${SOURCES})
