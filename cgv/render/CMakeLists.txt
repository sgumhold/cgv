cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

# Sources from PPP lib
#file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../../libs/ppp/*.cxx")
# Headers from PPP lib
#file(GLOB_RECURSE PPP_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "../../libs/ppp/*.h")


# Definition of the component
cgv_add_core_component(render
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	#${PPP_HEADERS}
	SOURCES ${SOURCES} 
	#${PPP_SOURCES}
	CGV_DEPENDENCIES utils type reflect data base signal math media ppp)

