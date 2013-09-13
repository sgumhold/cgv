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
	reflect
	render
	signal
	type
	utils)


# At the moment the CMake build system is not official but the in-house
# solution by Stefan Gumhold is used. In order to stick to standard procedures
# and make the framework compilable under linux CMake is used in parallel.
# However, this build system is more or less unofficial and the CMakeLists.txt
# files are not actively maintained by the main developer. Normally it is good
# practice to explicitely name all source and header files. But because the files
# to be compiled change and no constistent update of the CMakeLists can be
# guaranteed a globbing for source files is incorporated in the sub dirs.


# Add a sub directory for each component
foreach(comp ${CGV_COMPONENTS})
	add_subdirectory(${comp})
endforeach()

# Create a target that depends on all components
add_custom_target(cgv)

# Prepend a "cgv_" on every component
set(CGV_COMPONENTS_EXP)
foreach(${comp} ${CGV_COMPONENTS})
	list(APPEND CGV_COMPONENTS_EXP cgv_${comp})
endforeach()

add_dependencies(cgv ${CGV_COMPONENTS})

set_target_properties(cgv PROPERTIES FOLDER "${FOLDER_NAME_TOPLEVEL}")

build_core_components_finish()
