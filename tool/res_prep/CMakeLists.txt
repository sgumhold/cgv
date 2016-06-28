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
cgv_add_executable(res_prep ${SOURCES})

set_target_properties(res_prep PROPERTIES COMPILE_DEFINITIONS CGV_FORCE_STATIC)
set_target_properties(res_prep PROPERTIES FOLDER "${FOLDER_NAME_TOOL}")

cgv_write_find_file(res_prep)
