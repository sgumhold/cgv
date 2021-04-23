cmake_minimum_required(VERSION 3.14)

project(glew)

find_package(OpenGL REQUIRED)

add_library(glew glew.c)
target_link_libraries(glew ${OPENGL_LIBRARIES})
target_compile_definitions(glew PRIVATE GLEW_BUILD)

install(TARGETS glew EXPORT glew DESTINATION ${CGV_3RD_LIB_DEST})
install(EXPORT glew DESTINATION ${CGV_3RD_LIB_DEST})
