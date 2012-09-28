# Make sure this listfile is not included directly
# The file cgvConfigGeneral will define the CGV_CONFIG_GENERAL variable
if (NOT CGV_CONFIG_GENERAL)
	message(FATAL ERROR "The macros script is not intended to be included. Please
			 insert this file into your project by using find_package(cgv)")
	return()
endif()

get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

# Include macros that deal with the creation of modules, config files etc
include("${BASE_PATH}/CreateConfigMacros.cmake")
