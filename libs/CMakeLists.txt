cmake_minimum_required(VERSION 2.6)
project(libs)

add_subdirectory(ppp)
add_subdirectory(cgv_gl)
add_subdirectory(glsu)

add_custom_target(libs)
add_dependencies(libs ppp_lib cgv_gl)

set_target_properties(libs PROPERTIES FOLDER "${FOLDER_NAME_TOPLEVEL}")


