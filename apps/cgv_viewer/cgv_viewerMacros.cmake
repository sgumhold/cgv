get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

macro(cgv_add_viewer name)
	set(LIBS "")
	set(STATIC_DEFS "")
	set(SHARED_DEFS "")
	foreach (mod ${ARGN})
		if (TARGET ${mod})
			set(LIBS ${LIBS} ${mod})
			get_target_property(MOD_LIB_SHARED ${mod} _SHARED_DEFINITIONS)
			get_target_property(MOD_LIB_STATIC ${mod} _STATIC_DEFINITIONS)
			set(STATIC_DEFS ${STATIC_DEFS} ${MOD_LIB_STATIC})
			set(SHARED_DEFS ${SHARED_DEFS} ${MOD_LIB_SHARED})
		else()
			find_package(${mod})
			if (${mod}_FOUND)
				set(LIBS ${LIBS} "${${mod}_LIBRARIES}")
				set(SHARED_DEFS ${SHARED_DEFS} "${mod}_SHARED_DEFINITIONS")
				set(STATIC_DEFS ${STATIC_DEFS} "${mod}_STATIC_DEFINITIONS")
			else()
				message(FATAL_ERROR "A project requested to create a viewer. The required module ${mod} could not be found!")
			endif()
		endif()
	endforeach()

	if (BUILD_SHARED_LIBS)
		include(rpavlik/CreateLaunchers)
		if (NOT TARGET ${name})
			add_custom_target(${name})
		endif()
		add_dependencies(${name} ${cgv_viewer_EXECUTABLE} ${LIBS})
		
		set(ARGS_LIST "")

		set(CONFIGS ${CMAKE_CONFIGURATION_TYPES})
		if (NOT CONFIGS)
			set(CONFIGS "Release;Debug")
		endif()		

		foreach(config ${CONFIGS})
			get_target_property(V_${config} ${cgv_viewer_EXECUTABLE} LOCATION_${config})
			set_target_properties(${name} PROPERTIES EXECUTE_LOCATION_${config} "${V_${config}}")
			
			set(CFG_ARGS "")
			set(LIB_PATHS ".")
			foreach(lib ${LIBS})
				if (NOT TARGET ${lib})
					message(FATAL_ERROR "Error generating viewer ${name}. One of the requested plugins does not seem to be a plugin.")
				endif()
				
				get_target_property(LOC ${lib} LOCATION_${config})
				file(RELATIVE_PATH LOC "${CMAKE_CURRENT_SOURCE_DIR}" "${LOC}")

				set(CFG_ARGS "${CFG_ARGS} plugin:\"${LOC}\"")

				get_filename_component(LOC_PATH "${LOC}" PATH)
				list(APPEND LIB_PATHS "${LOC_PATH}")

			endforeach()
			list(APPEND ARGS_LIST "ARGS_${config};${CFG_ARGS}")
			set(ARGS_${config} ${CFG_ARGS})
		endforeach()

		list(REMOVE_DUPLICATES LIB_PATHS)

		create_target_launcher( 
			${name} 
			${ARGS_LIST}
			RUNTIME_LIBRARY_DIRS ${LIB_PATHS})			
	else()
		set(VIEWERNAME ${name})
		# FIXME: If the viewer name equals the name of a plugin then do not create a new target, but:
		# 1. Change the target to be an executable
		# 2. Include the main.cxx file into its sources
		if (TARGET ${name})
			set(VIEWERNAME ${name}_viewer)
		endif()
		
		add_executable(${VIEWERNAME} ${CGV_DIR}/apps/cgv_viewer/main.cxx)

		set(STATIC_DEFS ${STATIC_DEFS} "CGV_FORCE_STATIC")
		list(REMOVE_DUPLICATES STATIC_DEFS)
		set_target_properties(${VIEWERNAME} PROPERTIES COMPILE_DEFINITIONS "${STATIC_DEFS}")
		
		# Linux needs the additional "--whole-archive" flag. Otherwise the libraries will not be linked
		# As there are no direct references in the viewer main file
		if (WIN32)
			target_link_libraries(${VIEWERNAME} ${LIBS})
		else()
			target_link_libraries(${VIEWERNAME} "-Wl,--whole-archive" ${LIBS} "-Wl,--no-whole-archive")
		endif()
	endif()
endmacro()
