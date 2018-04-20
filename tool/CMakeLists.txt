cmake_minimum_required(VERSION 2.6)
project(tools)

add_subdirectory(ppp_tool)
add_subdirectory(shader_test)
add_subdirectory(res_prep)

add_custom_target(tools)
add_dependencies(tools ppp shader_test)

set_target_properties(tools PROPERTIES FOLDER "${FOLDER_NAME_TOPLEVEL}")


