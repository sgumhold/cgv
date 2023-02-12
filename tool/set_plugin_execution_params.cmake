
function(set_plugin_execution_params target_name)
	cmake_parse_arguments(
		PARSE_ARGV 1 CGVARG_ "" "ALTERNATIVE_COMMAND" "ARGUMENTS"
	)

	# command name
	if (CGVARG__ALTERNATIVE_COMMAND)
		set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_COMMAND ${CGVARG__ALTERNATIVE_COMMAND})
		set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_EXECUTABLE ${CGVARG__ALTERNATIVE_COMMAND})
	else()
		set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_COMMAND $<TARGET_FILE:cgv_viewer>)
		set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_EXECUTABLE $<TARGET_FILE:cgv_viewer>)
	endif()

	# command arguments
	set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${CGVARG__ARGUMENTS}")
	set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_ARGUMENTS "${CGVARG__ARGUMENTS}")
endfunction()

function(set_plugin_execution_working_dir target_name path)
	set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${path}")
	set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_WORKING_DIRECTORY "${path}")
endfunction()
