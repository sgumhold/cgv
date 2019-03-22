cmake_minimum_required(VERSION 2.6)
project(plot)

# The CGV framework is needed
find_package(cgv COMPONENTS utils type reflect data base media os gui render)

find_package(cgv_gl)

set(CMAKE_CXX_FLAGS "-std=c++11")

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Shader files
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

include_directories(
	"${CGV_DIR}/libs"
	"${cgv_INCLUDE_DIRS}"
	"${cgv_gl_INCLUDE_DIRS}"
	)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
cgv_add_library(plot 
	SOURCES ${SOURCES} 
	PUBLIC_HEADERS ${HEADERS}
	SHADERS ${SHADERS}
	DEFINITIONS UNICODE _UNICODE
	SHARED_DEFINITIONS PLOT_EXPORTS
	STATIC_DEFINITIONS PLOT_FORCE_STATIC)

target_link_libraries(plot ${cgv_LIBRARIES} ${cgv_gl_LIBRARIES})

cgv_write_find_file(plot)

