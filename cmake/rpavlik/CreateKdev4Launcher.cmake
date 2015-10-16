# Checks if the launch configuration was already inserted and if not
# inserts the configuration name into the file data
macro(_launcher_kdev_add_launch_config)
	set(_kdev_launchers_tag "[Launch]")
	set(_kdev_launchers_var "Launch Configurations=")

	# Find the line that contains the launcher configurations
	list(FIND KDEV_CONTENT "${_kdev_launchers_tag}" _kdev_launch_index)

	# If such a line was found then check the next line
	if (NOT _kdev_launch_index EQUAL -1)
		# Add 1 to the index of line [Launch] and get that line
		math(EXPR _kdev_launch_index "${_kdev_launch_index}+1")
		list(GET KDEV_CONTENT ${_kdev_launch_index} _kdev_launch_configs)

		# Check if this line really contains the run configurations
		string(FIND "${_kdev_launch_configs}" "${_kdev_launchers_var}" _kdev_launch_pos)
		if (NOT _kdev_launch_pos EQUAL -1)
			# Remember this line for later use and then remove it
			# But only do this if it contains more than just "Launch Configurations="
			string(LENGTH "${_kdev_launchers_var}" _kdev_var_len)
			string(LENGTH "${_kdev_launch_configs}" _kdev_conf_len)
			if (NOT (_kdev_var_len EQUAL _kdev_conf_len))
				set(_kdev_present_config "${_kdev_launch_configs}")
			endif()

			# This is the right line, now check if our configuration is already there
			string(FIND "${_kdev_launch_configs}" "CGVLaunch${CMAKE_BUILD_TYPE}" _kdev_launch_pos)
			# If we found this config then there is nothing more to do
			if (NOT _kdev_launch_pos EQUAL -1)
				set(_kdev_launcher_present 1)
				message("KDevelop starter found")
			endif()
		endif()
	endif()

	# Check if the last launcher was sane
	# TODO

	# Only proceed if no launcher entry was found
	if (NOT DEFINED _kdev_launcher_present)

		# If no [Launch] was provided so far then add one at the end of the file
		# And set the index to the next line
		if (_kdev_launch_index EQUAL -1)
			list(APPEND KDEV_CONTENT "${_kdev_launchers_tag}")
			list(LENGTH KDEV_CONTENT _kdev_launch_index)
			list(APPEND KDEV_CONTENT "")
		endif()

		# Initialize the launch configurations line
		# If one exists then add a comma, else create a new line
		if (DEFINED _kdev_present_config)
			set(_kdev_config "${_kdev_present_config},")
		else()
			set(_kdev_config "${_kdev_launchers_var}")
		endif()

		# Add our new configuration
		set(_kdev_config "${_kdev_config}CGVLaunch${CMAKE_BUILD_TYPE}")

		# Replace the line
		list(INSERT KDEV_CONTENT ${_kdev_launch_index} "${_kdev_config}") 
		math(EXPR _kdev_launch_index "${_kdev_launch_index}+1")
		list(REMOVE_AT KDEV_CONTENT ${_kdev_launch_index})
	endif()
endmacro()





macro(_launcher_kdev_get_deps_string)
	set(KDEV_DEPS_STRING "@Variant(\\x00\\x00\\x00\\t\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x0b\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x08")

	set(_launcher_deps_index "0")
	set(_launcher_deps_finished)

	string(LENGTH ${_targetname} _launcher_deps_len)

	# Go through every character of the target name
	while(NOT DEFINED _launcher_deps_finished)

		string(SUBSTRING "${_targetname}" ${_launcher_deps_index} 1 _launcher_deps_char)

		set(KDEV_DEPS_STRING "${KDEV_DEPS_STRING}\\x00${_launcher_deps_char}")

		math(EXPR _launcher_deps_index "${_launcher_deps_index}+1")
		if (NOT (_launcher_deps_index LESS _launcher_deps_len))
			set(_launcher_deps_finished 1)
		endif()	

	endwhile()

	# Add the final closing bracket
	set(KDEV_DEPS_STRING "${KDEV_DEPS_STRING})")
endmacro()





macro(_launcher_kdev_add_launch_data)
	# Read the launcher template
	file(READ "${_launchermoddir}/kdevlauncher.in" _kdev_launcher)

	# Extract relevant variables
	get_target_property(USERFILE_COMMAND ${_targetname} EXECUTE_LOCATION_${CMAKE_BUILD_TYPE})

	if (NOT USERFILE_COMMAND)
		get_target_property(USERFILE_COMMAND ${_targetname} LOCATION_${CMAKE_BUILD_TYPE})
	endif()

	set(LAUNCHERSCRIPT_COMMAND_ARGUMENTS "${ARGS_${CMAKE_BUILD_TYPE}} ${FWD_ARGS}")
	file(TO_NATIVE_PATH "${USERFILE_COMMAND}" USERFILE_COMMAND)


	# Set variables
	set(KDEV_LAUNCHER_ID "CGVLaunch${CMAKE_BUILD_TYPE}")
	set(KDEV_LAUNCHER_NAME "Run Viewer (${CMAKE_BUILD_TYPE})")
	set(KDEV_LAUNCHER_ARGUMENTS ${LAUNCHERSCRIPT_COMMAND_ARGUMENTS})
	set(KDEV_LAUNCHER_EXECUTABLE ${USERFILE_COMMAND})
	set(KDEV_LAUNCHER_WORKING_DIR ${USERFILE_WORKING_DIRECTORY})
	_launcher_kdev_get_deps_string()
	string(CONFIGURE "${_kdev_launcher}" _kdev_conf_launcher @ONLY)

	# Add this to the kdev config file
	set(KDEV_CONTENT ${KDEV_CONTENT} ${_kdev_conf_launcher})
endmacro()





macro(_launcher_produce_kdevelop_config)
	# Check if the KDevelop flag was set
	if(KDEVELOP)
		set(KDEV_FILE "${CMAKE_SOURCE_DIR}/.kdev4/${_targetname}.kdev4")

		# Check if the KDevelop configuration file and open it if neccessary
		if (EXISTS "${KDEV_FILE}")
			file(STRINGS "${KDEV_FILE}" KDEV_CONTENT)
		endif()

		# Check if a launch section is present in the config file. If not, insert one;
		# if yes, add an entry for us IF this element is not already our launcher
		_launcher_kdev_add_launch_config()

		# Only proceed if the launcher configuration is not already in the file
		# The variable _kdev_launcher_present is set by the upper macro
		if (NOT DEFINED _kdev_launcher_present)

			# Backup the old file
			if (EXISTS "${KDEV_FILE}")
				set(KDEV_CONTENT_NEWLINES)
				foreach(cline ${KDEV_CONTENT})
					list(APPEND KDEV_CONTENT_NEWLINES "${cline}\n")
				endforeach()

				file(WRITE "${KDEV_FILE}.bak" ${KDEV_CONTENT_NEWLINES})
			endif()

			# Insert the new launcher
			_launcher_kdev_add_launch_data()

			# Save the new version
			set(KDEV_CONTENT_NEWLINES)
			foreach(cline ${KDEV_CONTENT})
				list(APPEND KDEV_CONTENT_NEWLINES "${cline}\n")
			endforeach()

			file(WRITE "${KDEV_FILE}" ${KDEV_CONTENT_NEWLINES})

		endif()
	endif()
endmacro()

