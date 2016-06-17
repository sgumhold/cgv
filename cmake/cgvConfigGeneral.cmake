# Extract the path of this file
get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Include the file that defines all sorts of definitions and paths
include("${BASE_PATH}/base.cmake")

# Inherit build settings if wanted
if (NOT cgv_FIND_IGNORE_SETTINGS)
	include("${BASE_PATH}/buildSettings.cmake")
endif()

# Set the CGV_DIR and CGV_INSTALL_DIR path
set(CGV_DIR "")

if (EXISTS "${CGV_BASE}/cmake/local_build_dir.cmake")
	include("${CGV_BASE}/cmake/local_build_dir.cmake")
#	include("${LOCAL_BUILD_DIR}/${INSTALL_CMAKE_PATH}/local_config.cmake")
	set(CGV_DIR "${CGV_BASE}")
	set(CGV_INSTALL_DIR "${LOCAL_BUILD_DIR}")
else()
	set(CGV_DIR "${CGV_BASE}/${INSTALL_HEADER_PATH}")
	set(CGV_INSTALL_DIR "${CGV_BASE}")
endif()

set(CMAKE_MODULE_PATH "${CGV_INSTALL_DIR}/${INSTALL_CMAKE_PATH};${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}")


set(EXPORTS_FILE_LOCATION "${CGV_INSTALL_DIR}/${INSTALL_CMAKE_PATH}/${CGV_EXPORTS_NAME}") 

# FIXME: Add a better check for already imported projects
if (NOT TARGET cgv_utils)
    if (NOT EXISTS "${EXPORTS_FILE_LOCATION}")
    	# TODO: Add a better error message and fail
        message(FATAL_ERROR "There is no CGV installation for this operating system.")
        return()
    endif()
	include("${EXPORTS_FILE_LOCATION}")
endif()


set(TARGETS_CONFIG_FILE "${CGV_INSTALL_DIR}/${INSTALL_CMAKE_PATH}/${CGV_TARGETS_CONFIG_NAME}")
if (EXISTS "${TARGETS_CONFIG_FILE}")
	include("${TARGETS_CONFIG_FILE}")
endif()


# Set the list of requested components. If the find_package command
# was called with the "COMPONENTS" parameter then the cgv_FIND_COMPONENTS
# variable is set. Otherwise use the list of all existing components
set(CGV_REQUESTED "")

if (DEFINED cgv_FIND_COMPONENTS AND NOT cgv_FIND_COMPONENTS STREQUAL "")
	foreach(comp ${cgv_FIND_COMPONENTS})
		list(APPEND CGV_REQUESTED cgv_${comp})
	endforeach()
else()
	set(CGV_REQUESTED ${CGV_TARGETS})
endif()


# Check if the requested targets exists and add its dependencies to a list
foreach(comp ${CGV_REQUESTED})
	if (NOT TARGET ${comp})
		# TODO: What to do here?
		message("ERROR: Could not find component ${comp}")
	endif()
endforeach()

# Set the libraries to be the imported targets where each target corresponds
# to a component that was specified by the user
set(cgv_LIBRARIES ${CGV_REQUESTED})

# Set the include directories
# FIXME: this might be a hack
set(cgv_INCLUDE_DIRS "${CGV_DIR}"
					"${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_HEADER_PATH}")


set(${PROJECT_NAME}_FIND_DEP_cgv "cgv COMPONENTS ${CGV_REQUESTED}")
set(${PROJECT_NAME}_FIND_DEPS "${PROJECT_NAME}_FIND_DEP_cgv")
set(${PROJECT_NAME}_DEP_NAMES "cgv")

