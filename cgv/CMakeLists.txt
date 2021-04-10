cmake_minimum_required(VERSION 3.14)

project(CGV)

# include("../cmake/buildCoreComponents.cmake")

# Define all components
set(CGV_COMPONENTS
	base
	data
	gui
	math
	media
	os
	ppp
	reflect
	render
	signal
	type
	utils
	
	# config
	# defines
)

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

# FIXME find out what these two lines used to do
# set_target_properties(cgv PROPERTIES FOLDER "${FOLDER_NAME_TOPLEVEL}")
# build_core_components_finish()
