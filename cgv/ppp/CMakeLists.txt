cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

set(PUBLIC_HEADERS
	command_token.h
	expression_processor.h
	lib_begin.h
	operators.h
	ph_processor.h
	variables.h
	variant.h)


set(SOURCES
	command_token.cxx
	expression_processor.cxx
	operators.cxx
	ph_processor.cxx
	ppp_variant.cxx
	variables.cxx
)

# Definition of the component
cgv_add_core_component(ppp
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${PUBLIC_HEADERS}
	CGV_DEPENDENCIES utils type)


