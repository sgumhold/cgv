cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Under windows we need pthread
#if(WIN32)
#	cgv_find_package(pthread)
#endif()


# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Files to be preprocessed by PPP
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")


# FIXME: Another hack
if (WIN32)
#	include_directories(../../3rd/pthread)
else()
	set(CMAKE_CXX_FLAGS "-std=c++11")
endif()	


# add defines with the cgv-directory and the renderer_test directory
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCGV_DIR=\"${CGV_SOURCE_DIR}\" -DINPUT_DIR=\"${CMAKE_CURRENT_LIST_DIR}\"")

# Definition of the component
cgv_add_core_component(os
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	SOURCES ${SOURCES}
	CGV_DEPENDENCIES utils
	SHARED_DEFINITIONS ${pthread_SHARED_DEFINITIONS}
	STATIC_DEFINITIONS ${pthread_STATIC_DEFINITIONS})
	
# Additional dependencies

# In windows we also need ws2_32
if (WIN32)
#	target_link_libraries(cgv_os ${pthread_LIBRARIES})
	target_link_libraries(cgv_os ws2_32)
else()
	target_link_libraries(cgv_os ${CMAKE_THREAD_LIBS_INIT})
endif()
