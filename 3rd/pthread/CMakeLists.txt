cmake_minimum_required(VERSION 2.6)
project(pthread)

# Include this file for standard build settings
include("../../cmake/buildCoreComponents.cmake")

set(HEADERS
	pthread.h)
	
set(SOURCES
	pthread.c)
	
cgv_add_3rd_library(pthread
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${HEADERS}
	DEFINITIONS "PTW32_BUILD"
	STATIC_DEFINITIONS "PTW32_STATIC_LIB")
		
if (WIN32)
	target_link_libraries(pthread ws2_32)
endif()
		
cgv_write_find_file(pthread)
