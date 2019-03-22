cmake_minimum_required(VERSION 2.6)
project(cg_ext)

# The CGV framework is needed
find_package(cgv COMPONENTS render gui utils type reflect data base signal HINTS $ENV{CGV_DIR})
cgv_find_package(cg_icons)

file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

include_directories(${cgv_INCLUDE_DIRS})

cgv_add_module(cg_ext ${SOURCES} ${HEADERS})

_cgv_set_definitions(cg_ext SHARED CGV_GUI_EXT_EXPORTS)

target_link_libraries(cg_ext  ${cgv_LIBRARIES})
set_target_properties(cg_ext PROPERTIES FOLDER "${FOLDER_NAME_PLUGINS}")

cgv_write_find_file(cg_ext) 

