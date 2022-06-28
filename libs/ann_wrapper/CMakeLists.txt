
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(ann_wrapper SOURCES ${SOURCES} DEPENDENCIES ann cgv_math)

target_compile_definitions(ann_wrapper PRIVATE ANN_WRAPPER_EXPORT)
