cmake_minimum_required(VERSION 2.6)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# The PPP is required
find_package(ppp)

# Public headers
# TODO: To be verified
set(PUBLIC_HEADERS
	action.h
	attach_slot.h
	base.h
	base_generator.h
	console.h
	group.h
	import.h
	lib_begin.h
	named.h
	node.h
	register.h
	traverser.h)

# Source files
set(SOURCES
	action.cxx
	attach_slot.cxx
	base.cxx
	base_generator.cxx
	console.cxx
	group.cxx
	import.cxx
	named.cxx
	node.cxx
	register.cxx
	traverser.cxx)

# Files to be preprocessed by the PPP
set(PPP_SOURCES
	call_interface_impl.ph)

# Schedule preprocessing commands
ppp_compile("${CGV_DIR}"
			PPP_FILES 
			${PPP_SOURCES})

# Definition of the component
cgv_add_core_component(base
					SOURCES ${SOURCES}
					HEADERS ${PPP_FILES}
					PUBLIC_HEADERS ${PUBLIC_HEADERS}
					CGV_DEPENDENCIES utils type data reflect signal)

# Additional dependencies
if (NOT WIN32)
	target_link_libraries(cgv_base dl)
endif()

