cmake_minimum_required(VERSION 2.6)
project(point_cloud)

# The CGV framework is needed
find_package(cgv COMPONENTS utils type reflect data base media os gui render)
find_package(cgv_gl)

# 3rd party libraries
cgv_find_package(ANNf)
cgv_find_package(GLEW)

set(CMAKE_CXX_FLAGS "-std=c++11")

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.c*")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Shader files
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

include_directories(
	"${CGV_DIR}/libs"
	"${cgv_INCLUDE_DIRS}"
	"${GLEW_INCLUDE_DIRS}"
)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
if(WIN32)
	cgv_add_library(point_cloud 
		SOURCES ${SOURCES} 
		PUBLIC_HEADERS ${HEADERS}
		SHADERS ${SHADERS}
		DEFINITIONS UNICODE _UNICODE
		SHARED_DEFINITIONS POINT_CLOUD_EXPORTS
		STATIC_DEFINITIONS REGISTER_SHADER_FILES)
else()
	cgv_add_library(point_cloud 
		SOURCES ${SOURCES} 
		PUBLIC_HEADERS ${HEADERS}
		SHADERS ${SHADERS}
		DEFINITIONS UNICODE _UNICODE
		SHARED_DEFINITIONS POINT_CLOUD_EXPORTS)
endif()

target_link_libraries(point_cloud ${cgv_LIBRARIES} ${cgv_gl_LIBRARIES} ANNf)

cgv_write_find_file(point_cloud)

