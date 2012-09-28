cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# The PPP is required
find_package(ppp)

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	debug_reflection_handler.h
	find_reflection_handler.h
	get_reflection_handler.h
	lib_begin.h
	reflect_enum.h
	reflect_extern.h
	reflection_handler.h
	reflection_traits.h
	reflection_traits_info.h
	self_reflection_tag.h
	set_reflection_handler.h)

# Source files
set(SOURCES
	debug_reflection_handler.cxx
	find_reflection_handler.cxx
	get_reflection_handler.cxx
	reflect_enum.cxx
	reflection_handler.cxx
	reflection_traits.cxx
	set_reflection_handler.cxx)

# Files to be preprocessed by the PPP
set(PPP_SOURCES
	method_interface_impl.ph)

# Schedule preprocessing commands
ppp_compile("${CGV_DIR}"
			PPP_FILES 
			${PPP_SOURCES})

# Definition of the component
cgv_add_core_component(reflect
	PUBLIC_HEADERS ${PUBLIC_HEADERS} ${PPP_FILES}
	SOURCES ${SOURCES}
	CGV_DEPENDENCIES utils type)

