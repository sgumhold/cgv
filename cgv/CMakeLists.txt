cmake_minimum_required(VERSION 2.6)
project(cgv)

include("../cmake/buildCoreComponents.cmake")

# Define all components
set(CGV_COMPONENTS
	base
	config
	data
	defines
	gui
	math
	media
	os
	ppp
	reflect
	render
	signal
	type
	utils)

# Add a sub directory for each component
foreach(comp ${CGV_COMPONENTS})
	add_subdirectory(${comp})
endforeach()

# Create a target that depends on all components
add_custom_target(cgv)

# Prepend a "cgv_" on every component
set(CGV_COMPONENTS_EXP)
foreach(comp ${CGV_COMPONENTS})
	list(APPEND CGV_COMPONENTS_EXP cgv_${comp})
endforeach()

add_dependencies(cgv ${CGV_COMPONENTS_EXP})

set_target_properties(cgv PROPERTIES FOLDER "${FOLDER_NAME_TOPLEVEL}")

build_core_components_finish()
