cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE PUBLIC_HEADERS_HH RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.hh")

# Definition of the component
cgv_add_core_component(media
	SOURCES ${SOURCES}
	HEADERS ${HEADERS} 
	PUBLIC_HEADERS ${PUBLIC_HEADERS} ${PUBLIC_HEADERS_HH}
	CGV_DEPENDENCIES utils type data base)

if (UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

# This component has multiple static and shared definitions
cgv_add_export_definitions(cgv_media CGV_MEDIA_FONT CGV_MEDIA_ILLUM CGV_MEDIA_IMAGE CGV_MEDIA_VIDEO)

