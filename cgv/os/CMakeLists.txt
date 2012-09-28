cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Under windows we need pthread
if(WIN32)
	cgv_find_package(pthread)
else()
	set(pthread_LIBRARIES "pthread")
endif()


# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	clipboard.h
	display.h
	http_request.h
	lib_begin.h
	mutex.h
	socket.h
	thread.h
	web_server.h)

# Source files
set(SOURCES
	clipboard.cxx
	display.cxx
	mutex.cxx
	pthread_lib.cxx
	socket.cxx
	thread.cxx
	web_server.cxx)

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
