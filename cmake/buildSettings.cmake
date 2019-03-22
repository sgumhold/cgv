# Only include the build settings once
if (BUILD_SETTINGS_INCLUDED)
	return()
endif()

set(BUILD_SETTINGS_INCLUDED TRUE)

# Set CMake policy 11 to have included scripts push and pop policies
cmake_policy(SET CMP0011 NEW)
# Set CMake policy 17 to prefer local find scripts
# cmake_policy(SET CMP0017 OLD)
# Set CMake policy 53 for old expansion rules
cmake_policy(SET CMP0053 OLD)


get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${BASE_PATH}/base.cmake")

# Set the CGV_DIR variable to be the base path. The includes are expected
# to be in the sub directory "cgv"
set(CGV_DIR "${CGV_BASE}")

# Add the include directory to the list of include directories
include_directories("${CGV_DIR}")

# Set some cmake variables to control the output location of compiled targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_BIN_PATH}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_BIN_PATH}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_LIB_PATH}")
set(CMAKE_MODULE_PATH 
	"${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}"
	"${CGV_DIR}/cmake"
	)
	
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
	
# Set names for the resulting binaries
if (BUILD_SHARED_LIBS)
	set(CMAKE_DEBUG_POSTFIX ${DEBUG_POSTFIX})
	set(CMAKE_REALWITHDEBINFO_POSTFIX ${CMAKE_DEBUG_POSTFIX})
else()
	set(CMAKE_DEBUG_POSTFIX "${STATIC_POSTFIX}${DEBUG_POSTFIX}")
	set(CMAKE_REALWITHDEBINFO_POSTFIX ${CMAKE_DEBUG_POSTFIX})
	set(CMAKE_RELEASE_POSTFIX "${STATIC_POSTFIX}")
	set(CMAKE_POSTFIX "${STATIC_POSTFIX}")
endif()

if (CGV_INSTALL_TO_GLOBAL)
	message("Setting installation directory to ${CGV_BASE}")
	set(CMAKE_INSTALL_PREFIX "${CGV_BASE}/..")
endif()

# Set the path for the CGV find_package file
set(cgv_DIR "${CGV_BASE}/cmake")

