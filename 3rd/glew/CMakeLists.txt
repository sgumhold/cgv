cmake_minimum_required(VERSION 2.6)
project(GLEW)

# Include this file for standard build settings
include("../../cmake/buildCoreComponents.cmake")

cgv_find_package(OpenGL)

set(SOURCES
	glew.c)

set(PUBLIC_HEADERS
	GL/glew.h
	GL/glxew.h
	GL/wglew.h)
	
include_directories(${OPENGL_INCLUDE_DIR})

cgv_add_3rd_library(GLEW
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	SHARED_DEFINITIONS "GLEW_BUILD"
	STATIC_DEFINITIONS "GLEW_STATIC")

target_link_libraries(GLEW
	${OPENGL_LIBRARIES})

cgv_write_find_file(GLEW)
