
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cg_ext ${SOURCES})
target_link_libraries(cg_ext cgv_base cgv_gui)
target_compile_definitions(cg_ext PRIVATE CGV_GUI_EXT_EXPORTS)
