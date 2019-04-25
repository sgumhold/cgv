include("../cmake/buildSettings.cmake")

# FIXME: Add support for versions
# TODO: Add support for libraries that don't have CMake scripts
macro(check_3rd_package pkgname subdir depslist)
	# Check if our own bastard version of the Findxxx file exists and delete
	# it if neccessary
	foreach(mpath ${CMAKE_MODULE_PATH})
		if (EXISTS "${mpath}/Find${pkgname}.cmake")
			file(REMOVE "${mpath}/Find${pkgname}.cmake")
		endif()
	endforeach()

	set(LOCAL_FIND "${CMAKE_CURRENT_SOURCE_DIR}/Find${pkgname}.cmake")
	# Check if there is a FindXXX.cmake in this directory
	if (EXISTS "${LOCAL_FIND}")
		# Include this file and let it run
		include("${LOCAL_FIND}")
		# Check if the package was found
		if (${pkgname}_FOUND)
			configure_file("${LOCAL_FIND}" "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}/Find${pkgname}.cmake" COPY_ONLY)
			install(FILES "${LOCAL_FIND}" DESTINATION "${INSTALL_BASE}/${INSTALL_CMAKE_PATH}")
			message(" * No find script for ${pkgname} globally found but a local script returned success")
			set(LOCAL_SUCCESS TRUE)
		endif()
	endif()

	if (NOT LOCAL_SUCCESS)
		find_package(${pkgname} QUIET)
		if (${pkgname}_FOUND)
			message(" * Found native installation for ${pkgname}")
			if (NOT ${pkgname}_INCLUDE_DIRS)
				set(${pkgname}_INCLUDE_DIRS ${pkgname}_INCLUDES ${pkgname}_INCLUDE_DIR ${pkgname}_DIR)
			endif()
			if (NOT ${pkgname}_LIBRARIES)
				set(${pkgname}_LIBRARIES ${pkgname}_LIBRARY ${pkgname}_LIB)
			endif()
		else()
			# Remove the failed cache entries
			set(${pkgname}_LIBRARY "" CACHE INTERNAL "hidden ${pkgname} library path")
			set(${pkgname}_DIR "" CACHE INTERNAL "hidden ${pkgname} dir")
			set(${pkgname}_INCLUDE_DIR "" CACHE INTERNAL "hidden ${pkgname} dir")
			# Add the local subdirectory
			add_subdirectory(${subdir})
			message(" * Could not find package ${pkgname}. Using local sources from ${subdir}")
			set(${depslist} "${${depslist}}" ${pkgname})
		endif()
	endif()
endmacro()


set(depslist "")

#check_3rd_package(ANN ann depslist)
check_3rd_package(ZLIB zlib depslist)
check_3rd_package(GLEW glew depslist)
# check_3rd_package(PNG png depslist)
check_3rd_package(JPEG jpeg depslist)
check_3rd_package(TIFF tiff depslist)
# check_3rd_package(FLTK2 fltk depslist)

# Always use our modified bastard version
add_subdirectory(ANN)
# add_subdirectory(glew)
add_subdirectory(png)
# add_subdirectory(jpeg)
add_subdirectory(fltk)
# add_subdirectory(tiff)
# add_subdirectory(zlib)
list(APPEND depslist "FLTK2")

# Add a custom target for all 3rd packages
add_custom_target(3rd)
set_target_properties(3rd PROPERTIES FOLDER "${FOLDER_NAME_TOPLEVEL}")

if (NOT "${depslist}" STREQUAL "")
	add_dependencies(3rd ${depslist})
endif()
