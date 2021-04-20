cmake_minimum_required(VERSION 3.14)

project(glew)

find_package(OpenGL REQUIRED)

add_library(glew glew.c)
target_include_directories(glew PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${OPENGL_INCLUDE_DIR})
target_link_libraries(glew ${OPENGL_LIBRARIES})
target_compile_definitions(glew PRIVATE GLEW_BUILD)
