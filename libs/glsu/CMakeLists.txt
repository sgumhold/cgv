cmake_minimum_required(VERSION 2.6)
project(glsu)

# Needed for cgv_ commands
find_package(cgv COMPONENTS)

cgv_find_package(OpenGL)

set(HEADERS GL/glsu.h)

set(SOURCES 
	glsu.c
)

# Also add the .def-file to the sources. CMake will know what to do with it
# FIXME: Do not check for WIN32, but for VisualStudio
if (WIN32)
	set(SOURCES ${SOURCES} glsu.def)
endif()

include_directories(${OPENGL_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}")

cgv_add_library(glsu 
	SOURCES ${SOURCES} 
	HEADERS ${HEADERS})

target_link_libraries(glsu ${OPENGL_LIBRARIES})


cgv_write_find_file(glsu)

