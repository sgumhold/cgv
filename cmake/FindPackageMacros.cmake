if (NOT CGV_INSTALL_DIR)
	# Check if a base file exists in the same path as this file
	# This is the case when the framework is installed
	if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/base.cmake")
		include("${CMAKE_CURRENT_LIST_DIR}/base.cmake")
	# Otherwise check if a base file exists in a predefined directory
	# structure that corresponds to the source tree of the framework
	elseif (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../cmake/base.cmake")
		include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/base.cmake")
	else()
		# TODO: Add a better error message
		message(FATAL_ERROR "Could not execute package finding script.")
	endif()
endif()



macro(cgv_find_element target)
	# the library is found in any case
	set(${targetname}_FOUND TRUE)

	# Set the path of the library
	# FIXME: This location will be invalid
	set(${target}_LIBRARIES "${CGV_INSTALL_DIR}/${INSTALL_LIB_PATH}/${target}")

	# Set the include directory of the library
	# FIXME: For installed libraries this path will be invalid
	set(${target}_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endmacro()



macro(cgv_find_binary target)
	# the library is found in any case
	set(${targetname}_FOUND TRUE)

	# Set the path of the library
	set(${target}_EXECUTABLE "${CGV_INSTALL_DIR}/${INSTALL_LIB_PATH}/${target}")

endmacro()

