cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	ascii_io_reflection_handlers.h
	binary_io_reflection_handlers.h
	component_format.h
	data_format.h
	data_view.h
	image_view.h
	io_reflection_handler.h
	lib_begin.h
	packing_info.h
	ref_arr.h
	ref_counted.h
	ref_ptr.h)

# Source files
set(SOURCES
	ascii_io_reflection_handlers.cxx
	binary_io_reflection_handlers.cxx
	component_format.cxx
	data_format.cxx
	data_view.cxx
	image_view.cxx
	io_reflection_handler.cxx
	packing_info.cxx)

# Definition of the component
cgv_add_core_component(data
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	CGV_DEPENDENCIES utils type reflect)

