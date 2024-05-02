
# global options list
set(CGV_OPTIONS_LIST "")

# declares a CMake option equivalent to some native PPP-based CGV option
function(cgv_define_option OPTION_NAME)
	cmake_parse_arguments(
		PARSE_ARGV 1 CGVARG_
		"" "DOC;DEFAULT" ""
	)

	# add new option to list
	set(CGV_OPTIONS_LIST_LOCAL ${CGV_OPTIONS_LIST})
	list(APPEND CGV_OPTIONS_LIST_LOCAL ${OPTION_NAME})

	# Override the default provided in the definition with any preset values
	# (e.g. from client projects using the framework)
	if (DEFINED CGV_${OPTION_NAME})
		set(ACTUAL_INIT_VALUE ${CGV_${OPTION_NAME}})
	else()
		set(ACTUAL_INIT_VALUE ${CGVARG__DEFAULT})
	endif()

	# propagate upwards
	set(CGV_OPTIONS_LIST ${CGV_OPTIONS_LIST_LOCAL} PARENT_SCOPE)
	set(CGV_${OPTION_NAME}_DOC ${CGVARG__DOC} PARENT_SCOPE)
	set(CGV_${OPTION_NAME}_INIT ${ACTUAL_INIT_VALUE} PARENT_SCOPE)
endfunction()

# handles declaring CGV options to CMake and initializing them
function(cgv_init_cgvoptions)
	# read current environment and prepare bookkeeping
	set(CGV_OPTIONS_ENV "$ENV{CGV_OPTIONS}")
	set(CGV_OPTIONS_LOCAL "")
	set(CGV_OPTIONS_FROM_SYSTEM FALSE)

	# handle each declared CGV option
	set(CGV_OPTIONS "")
	# - check for each option whether it needs initializing and from where
	foreach(CGV_OPTION ${CGV_OPTIONS_LIST})
		if (NOT DEFINED CGV_${CGV_OPTION})
			if (DEFINED ENV{CGV_OPTIONS})
				if (${CGV_OPTION} IN_LIST CGV_OPTIONS_ENV)
					set(CGV_${CGV_OPTION}_INIT ON)
				else()
					set(CGV_${CGV_OPTION}_INIT OFF)
				endif()
				set(CGV_OPTIONS_FROM_SYSTEM TRUE)
				list(APPEND CGV_OPTIONS_LOCAL "${CGV_OPTION}=${CGV_${CGV_OPTION}_INIT}")
			endif()
		endif()
		option(CGV_${CGV_OPTION} ${CGV_${CGV_OPTION}_DOC} ${CGV_${CGV_OPTION}_INIT})
		if (CGV_${CGV_OPTION})
			list(APPEND CGV_OPTIONS "${CGV_OPTION}")
		endif()
	endforeach()
	# - make sure the resulting CGV_OPTIONS string is available to any and all parent project(s) using the CGV Framework
	set(CGV_OPTIONS ${CGV_OPTIONS} CACHE INTERNAL
	    "The CGV_OPTIONS string resulting from the chosen CMake options (for overriding local tool environments)" FORCE)

	# print summary of options applied from system-wide configuration, if any
	if (CGV_OPTIONS_FROM_SYSTEM)
		message("---------------------------------------------------------")
		message(" Following CGV options were initialized with values")
		message(" from the system-wide CGV configuration:")
		foreach(CGV_OPTION ${CGV_OPTIONS_LOCAL})
			# print enabled options in default color, and disabled ones in highlight color
			if (CGV_${CGV_OPTION} MATCHES "[oO][nN]$")
				message(STATUS "   ${CGV_OPTION}")
			else()
				message("   ${CGV_OPTION}")
			endif()
		endforeach()
		message("---------------------------------------------------------")
	endif()

	# schedule an info message about enabled options after first configuration pass is done
	cmake_language(
		DEFER DIRECTORY ${CGV_DIR}
		CALL message STATUS "Using CGV_OPTIONS string: \"${CGV_OPTIONS}\""
	)
endfunction()
