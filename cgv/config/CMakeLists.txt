cmake_minimum_required(VERSION 2.8)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")


set(PUBLIC_HEADERS
	cpp_version.h
	lib_begin.h
	lib_end.h)

cgv_add_core_component(config
	HEADERS_ONLY
	PUBLIC_HEADERS ${PUBLIC_HEADERS})
