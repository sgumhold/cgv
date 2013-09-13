cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Under windows we need pthread
if(WIN32)
	cgv_find_package(pthread)
else()
	set(pthread_LIBRARIES "pthread")
endif()


# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Files to be preprocessed by PPP
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")


# FIXME: Another hack
if (WIN32)
	include_directories(../../3rd/pthread)
endif()	

# Definition of the component
cgv_add_core_component(os
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	SOURCES ${SOURCES}
	CGV_DEPENDENCIES utils
	SHARED_DEFINITIONS ${pthread_SHARED_DEFINITIONS}
	STATIC_DEFINITIONS ${pthread_STATIC_DEFINITIONS})
	
# Additional dependencies
target_link_libraries(cgv_os ${pthread_LIBRARIES})

# In windows we also need ws2_32
if (WIN32)
	target_link_libraries(cgv_os ws2_32)
endif()
