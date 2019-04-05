cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# The PPP is required
#find_package(ppp)

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# PPP files
#file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")


# Files to be preprocessed by the PPP
#set(PPP_SOURCES
#	call_interface_impl.ph)

# Schedule preprocessing commands
#ppp_compile("${CGV_DIR}"
#			PPP_FILES 
#			${PPP_SOURCES})

# Definition of the component
cgv_add_core_component(base
					SOURCES ${SOURCES}
#					HEADERS ${PPP_FILES}
					PUBLIC_HEADERS ${PUBLIC_HEADERS}
					CGV_DEPENDENCIES utils type data reflect signal)

# Additional dependencies
if (NOT WIN32)
	target_link_libraries(cgv_base dl)
endif()

