cmake_minimum_required(VERSION 2.6)
project(res_prep)

# Needed for building components
include("../../cmake/buildCoreComponents.cmake")

# Set source files
set(SOURCES
	res_prep.cxx
	${CGV_DIR}/cgv/utils/scan.cxx
    ${CGV_DIR}/cgv/utils/dir.cxx
    ${CGV_DIR}/cgv/utils/file.cxx)
	
# Set the project target
cgv_add_tool(res_prep ${SOURCES})

cgv_write_find_file(res_prep)
