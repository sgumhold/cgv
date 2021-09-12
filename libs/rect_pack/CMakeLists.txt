
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx" "*.cpp")

cgv_create_lib(rect_pack SOURCES ${SOURCES})

target_compile_definitions(rect_pack PRIVATE CGV_DIR)
target_compile_definitions(rect_pack_static PRIVATE CGV_DIR)
