cmake_minimum_required(VERSION 2.6)
project(libs)

add_subdirectory(glsu)
add_subdirectory(delaunay)

add_subdirectory(cgv_reflect_types)
add_subdirectory(cgv_gl)
add_subdirectory(plot)
add_subdirectory(point_cloud)

add_custom_target(libs)
add_dependencies(libs cgv_gl glsu plot cgv_reflect_types delaunay)

set_target_properties(libs PROPERTIES FOLDER "${FOLDER_NAME_LIBS}")

