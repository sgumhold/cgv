cmake_minimum_required(VERSION 2.6)
project(generate_makefiles)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Set source files
set(SOURCES generate_makefiles.cxx)
	
# Set the project target
cgv_add_executable(generate_makefiles ${SOURCES})

set_target_properties(ppp PROPERTIES FOLDER "${FOLDER_NAME_TOOL}")

cgv_write_find_file(generate_makefiles)
