cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# The PPP is required
find_package(ppp)

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	bool_combiner.h
	callback_stream.h
	lib_begin.h)

# Source files
set(SOURCES
	abst_signal.cxx
	bool_combiner.cxx)

# Files to be preprocessed by the PPP
set(PPP_SOURCES
	abst_signal.ph
	bool_signal.ph
	rebind.ph
	signal.ph)

# Schedule preprocessing commands
ppp_compile("${CGV_DIR}"
			PPP_FILES 
			${PPP_SOURCES})

# Definition of the component
cgv_add_core_component(signal
	CGV_DEPENDENCIES type
	PUBLIC_HEADERS ${PUBLIC_HEADERS} ${PPP_FILES}
	SOURCES ${SOURCES})

