cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# The PPP is required
find_package(ppp)


# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Files to be preprocessed by PPP
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")


# Create the PPP files
ppp_compile("${CGV_DIR}"
			PPP_FILES 
			${PPP_SOURCES})

# Definition of the component
cgv_add_core_component(utils
	PUBLIC_HEADERS ${PUBLIC_HEADERS} ${PPP_FILES}
	SOURCES ${SOURCES})

