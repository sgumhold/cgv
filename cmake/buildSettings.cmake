# Only include the build settings once
if (BUILD_SETTINGS_INCLUDED)
	return()
endif()

set(BUILD_SETTINGS_INCLUDED TRUE)


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


# Set the path for the CGV find_package file
set(cgv_DIR "${CGV_BASE}/cmake")

