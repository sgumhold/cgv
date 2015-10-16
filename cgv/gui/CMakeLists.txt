cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")


# Definition of the component
cgv_add_core_component(gui
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	CGV_DEPENDENCIES utils type reflect data base media signal)

