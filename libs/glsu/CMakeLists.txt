cmake_minimum_required(VERSION 3.14)

project(glsu)

add_library(glsu glsu.c glsu.def)
target_include_directories(glsu PUBLIC
	${OPENGL_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(glsu ${OPENGL_LIBRARIES})
