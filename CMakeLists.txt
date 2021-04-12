cmake_minimum_required(VERSION 3.14)

project(CGV)

option(BUILD_SHARED_LIBS "Create shared libraries" TRUE)
option(BUILD_EXAMPLE_PLUGIN "Build the example plugin" TRUE)

set(CGV_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(3rd)
add_subdirectory(tool)
add_subdirectory(cgv)
add_subdirectory(libs)
add_subdirectory(apps)
add_subdirectory(plugins)

# The framework depends on the tools and on 3rd libraries
# add_dependencies(cgv tools 3rd)
# The tools depend on the 3rd libraries
# add_dependencies(tools 3rd)
# The apps depend on the CGV framework and 3rd party libs
# add_dependencies(apps cgv 3rd)
# The plugins depend on everything
# add_dependencies(plugins cgv tools 3rd)
