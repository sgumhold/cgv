cmake_minimum_required(VERSION 2.6)
project(cgv_reflect_types)

# The CGV framework is needed!
find_package(cgv COMPONENTS  utils type reflect media math render gui)


# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

include_directories(
	"${CGV_DIR}/libs"
	${cgv_INCLUDE_DIRS})

cgv_add_library(cgv_reflect_types 
	SOURCES ${SOURCES} 
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	DEFINITIONS UNICODE _UNICODE
	SHARED_DEFINITIONS CGV_REFLECT_TYPES_EXPORTS)

	
target_link_libraries(cgv_reflect_types ${cgv_LIBRARIES})

cgv_write_find_file(cgv_reflect_types)

