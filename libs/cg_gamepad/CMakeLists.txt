cmake_minimum_required(VERSION 2.6)
project(cg_gamepad)

# -----------------------------------------------------------------------------
## CGV Framework ##
# The CGV framework is needed
find_package(cgv COMPONENTS utils type reflect data signal os base  math media gui HINTS ${CGV_DIR} )

# -----------------------------------------------------------------------------
## Dependencies from CGV Framework ##
cgv_find_package(cgv_reflect_types) 
cgv_find_package(gamepad REQUIRED)

# -----------------------------------------------------------------------------
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CGV_DIR}/libs
	${cgv_INCLUDE_DIRS}
	${cgv_reflect_types_INCLUDE_DIRS}
	${gamepad_INCLUDE_DIRS}
)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
cgv_add_library(cg_gamepad 
	SOURCES ${SOURCES} 
	PUBLIC_HEADERS ${HEADERS}
	DEFINITIONS UNICODE _UNICODE
	SHARED_DEFINITIONS CG_GAMEPAD_EXPORTS)

add_dependencies(cg_gamepad gamepad)
	
target_link_libraries(cg_gamepad 
	${cgv_LIBRARIES}
	${cgv_reflect_types_LIBRARIES} 
	${gamepad_LIBRARIES}
)

cgv_write_find_file(cg_gamepad)

