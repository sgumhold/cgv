cmake_minimum_required(VERSION 3.14)

project(cgv)

option(BUILD_EXAMPLE_PLUGIN "Build the example plugin" ON)
set(BUILD_SHARED_LIBS ON)

set(CGV_DIR ${PROJECT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin PARENT_SCOPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib PARENT_SCOPE)
endif()

add_subdirectory(3rd)
add_subdirectory(tool)
add_subdirectory(cgv)
add_subdirectory(libs)
add_subdirectory(apps)
add_subdirectory(plugins)
