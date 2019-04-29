get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

include("${cgv_DIR}/CMakeMacroParseArguments.cmake")

function(cgv_get_viewer_locations RELEASE_LOCATION DEBUG_LOCATION)
	get_target_property(RELEASE_LOC ${cgv_viewer_EXECUTABLE} LOCATION_Release)
	set(${RELEASE_LOCATION} ${RELEASE_LOC} PARENT_SCOPE)
	get_target_property(DEBUG_LOC ${cgv_viewer_EXECUTABLE} LOCATION_Debug)
	set(${DEBUG_LOCATION} ${DEBUG_LOC} PARENT_SCOPE)
endfunction()

function(cgv_add_viewer name)
	# We only support linux and shared library builds
	if (WIN32)
		message("Can only create a viewer for linux shared libraries at the moment")
		return()
	endif()

	if (NOT BUILD_SHARED_LIBS)
		message("Can only create a viewer for shared libraries at the moment")
		return()
	endif()

	_cgv_add_shared_viewer(${name} ${ARGN}) 
endfunction()




function(_cgv_add_shared_viewer name)
	set(CONFIG ${CMAKE_BUILD_TYPE})
	if (NOT CONFIG)
		set(CONFIG "Release")
	endif()
	cmake_parse_arguments(VDEFS "" "" "PLUGINS;ARGS" ${ARGN})
	# If the command was not invoked with PLUGINS and CMDLINE sub lists
	# then treat all arguments as plugins
	if (NOT VDEFS_PLUGINS AND NOT VDEFS_ARGS)
		set(VDEFS_PLUGINS ${ARGN})
		set(VDEFS_ARGS "")
	endif()

	# Collect all library targets and definitions for all plugins
	set(LIBS "")
	foreach (mod ${VDEFS_PLUGINS})
		# Check the according member if the plugin is a target 
		if (TARGET ${mod})
			set(LIBS ${LIBS} ${mod})
			get_target_property(MOD_LIB_SHARED ${mod} _SHARED_DEFINITIONS)
		# Otherwise find the package
		else()
			find_package(${mod})
			if (${mod}_FOUND)
				set(LIBS ${LIBS} "${${mod}_LIBRARIES}")
			else()
				message(FATAL_ERROR "A project requested to create a viewer. The required module ${mod} could not be found!")
			endif()
		endif()
	endforeach()


	set(LIB_FILES "")
	set(LIB_PATHS "")

	# Find the file name of the library targets and construct a command line
	# Also find the paths which we need for LD_LIBRARY_PATH later
	foreach(lib ${LIBS})
		get_target_property(LOC ${lib} LOCATION_${CONFIG})
		set(LIB_FILES "${LIB_FILES} plugin:\"${LOC}\"")

		get_filename_component(ABS_LOC_PATH "${LOC}" PATH)
		list(APPEND LIB_PATHS ${ABS_LOC_PATH})
	endforeach()
	list(REMOVE_DUPLICATES LIB_PATHS)
	string(REPLACE ";" ":" LIB_PATHS "${LIB_PATHS}")

	# Find the name of the cgv viewer
	get_target_property(VIEWER_EXE ${cgv_viewer_EXECUTABLE} LOCATION_${CONFIG})

	configure_file(${BASE_PATH}/launcher.sh.in
				   ${CMAKE_BINARY_DIR}/${name}.sh
				   @ONLY)
endfunction()


