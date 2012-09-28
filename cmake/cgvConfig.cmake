# Extract the path of this file
get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Check if we already processed this file
if (DEFINED CGV_CONFIG_PROCESSED)
	return()
endif()
set(CGV_CONFIG_PROCESSED "")

# Do the general setup
include("${BASE_PATH}/cgvConfigGeneral.cmake")

# Define macros
include("${BASE_PATH}/cgvConfigMacros.cmake")
