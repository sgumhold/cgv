
function(format_vscode_launch_json_args CONTENT_VAR)
	cmake_parse_arguments(PARSE_ARGV 1 CGVARG_ "" "" "ARGUMENT_LIST")

	if (CGVARG__ARGUMENT_LIST)
		set(LIST_CONTROL_HELPER TRUE) # <-- for distinguishing the first iteration within a foreach()
		# generate JSON list of command line arguments
		set(LOCAL_JSON_STRING "			\"args\": [")
		foreach(ARG IS_FIRST IN ZIP_LISTS CGVARG__ARGUMENT_LIST LIST_CONTROL_HELPER)
			string(REGEX REPLACE "\"" "\\\\\"" ARG_QE ${ARG})
			if (IS_FIRST)
				set(LOCAL_JSON_STRING "${LOCAL_JSON_STRING}\n				\"${ARG_QE}\"")
			else()
				set(LOCAL_JSON_STRING "${LOCAL_JSON_STRING},\n				\"${ARG_QE}\"")
			endif()
		endforeach()
		# append to output variable
		set(${CONTENT_VAR} "${${CONTENT_VAR}},\n${LOCAL_JSON_STRING}\n			]" PARENT_SCOPE)
	endif()
endfunction()

function(format_vscode_launch_json_entry CONTENT_VAR TARGET_NAME)
	cmake_parse_arguments(
		PARSE_ARGV 2 CGVARG_ "" "DEBUGGER_TYPE;LAUNCH_PROGRAM;WORKING_DIR" "CMD_ARGS"
	)

	# format JSON list entry
	# - check if we're generating the first entry
	set(CONTENT_LOCAL "")
	if (${CONTENT_VAR} AND NOT ${CONTENT_VAR} STREQUAL "")
		set(CONTENT_LOCAL ",\n")
	endif()
	# - generate entry content
	if (CGVARG__DEBUGGER_TYPE STREQUAL "cppdbg")
		set(CONTENT_LOCAL "${CONTENT_LOCAL}		{
			\"name\": \"Debug '${TARGET_NAME}' (cppdbg)\",
			\"type\": \"cppdbg\",
			\"request\": \"launch\",
			\"program\": \"${CGVARG__LAUNCH_PROGRAM}\"")
		format_vscode_launch_json_args(CONTENT_LOCAL ARGUMENT_LIST ${CGVARG__CMD_ARGS})
		set(CONTENT_LOCAL "${CONTENT_LOCAL},
			\"cwd\": \"${CGVARG__WORKING_DIR}\",
			\"environment\": [
				{ \"name\": \"CGV_DIR\", \"value\": \"${CGV_DIR}\" },
				{ \"name\": \"CGV_OPTIONS\", \"value\": \"${CGV_OPTIONS}\" }
			],
			\"externalConsole\": false,
			\"MIMode\": \"gdb\",
			\"setupCommands\": [
				{
					\"description\": \"Use pretty printing of variables and containers\",
					\"text\": \"-enable-pretty-printing\",
					\"ignoreFailures\": true
				},
				{
					\"description\": \"Use Intel-style disassembly\",
					\"text\": \"-gdb-set disassembly-flavor intel\",
					\"ignoreFailures\": true
				}
			]
		}")
	elseif (CGVARG__DEBUGGER_TYPE STREQUAL "CodeLLDB")
		set(CONTENT_LOCAL "${CONTENT_LOCAL}		{
			\"name\": \"Debug '${TARGET_NAME}' (CodeLLDB)\",
			\"type\": \"lldb\",
			\"request\": \"launch\",
			\"program\": \"${CGVARG__LAUNCH_PROGRAM}\"")
		format_vscode_launch_json_args(CONTENT_LOCAL ARGUMENT_LIST ${CGVARG__CMD_ARGS})
		set(CONTENT_LOCAL "${CONTENT_LOCAL},
			\"cwd\": \"${CGVARG__WORKING_DIR}\",
			\"env\": { \"CGV_DIR\": \"${CGV_DIR}\", \"CGV_OPTIONS\": \"${CGV_OPTIONS}\" }
		}")
	else()
		message(FATAL_ERROR "format_vscode_launch_json_entry(): unknown debugger type \"${CGVARG__DEBUGGER_TYPE}\"")
	endif()

	# propagate result
	set(${CONTENT_VAR} "${${CONTENT_VAR}}${CONTENT_LOCAL}" PARENT_SCOPE)
endfunction()

function(concat_vscode_launch_json_content LAUNCH_JSON_CONFIG_VAR TARGET_NAME)
	cmake_parse_arguments(
		PARSE_ARGV 2 CGVARG_ "NO_EXECUTABLE" "WORKING_DIR" "PLUGIN_ARGS;EXE_ARGS"
	)

	# determine target names
	cgv_get_static_or_exe_name(NAME_STATIC NAME_EXE ${TARGET_NAME} TRUE)

	# compose JSON list for both plugin and executable variants
	# - plugin build, standard VS Code C++ debugging
	format_vscode_launch_json_entry(JSON_LIST_STRING
		${TARGET_NAME} DEBUGGER_TYPE cppdbg
		LAUNCH_PROGRAM $<TARGET_FILE:cgv_viewer> CMD_ARGS ${CGVARG__PLUGIN_ARGS}
		WORKING_DIR ${CGVARG__WORKING_DIR}
	)
	# - plugin build, CodeLLDB debugging
	format_vscode_launch_json_entry(JSON_LIST_STRING
		${TARGET_NAME} DEBUGGER_TYPE CodeLLDB
		LAUNCH_PROGRAM $<TARGET_FILE:cgv_viewer> CMD_ARGS ${CGVARG__PLUGIN_ARGS}
		WORKING_DIR ${CGVARG__WORKING_DIR}
	)
	# - single executable build if not disabled
	if(NOT CGVARG__NO_EXECUTABLE)
		# standard VS Code C++ debugging
		format_vscode_launch_json_entry(JSON_LIST_STRING
			${NAME_EXE} DEBUGGER_TYPE cppdbg
			LAUNCH_PROGRAM $<TARGET_FILE:${NAME_EXE}> CMD_ARGS ${CGVARG__EXE_ARGS}
			WORKING_DIR ${CGVARG__WORKING_DIR}
		)
		# CodeLLDB debugging
		format_vscode_launch_json_entry(JSON_LIST_STRING
			${NAME_EXE} DEBUGGER_TYPE CodeLLDB
			LAUNCH_PROGRAM $<TARGET_FILE:${NAME_EXE}> CMD_ARGS ${CGVARG__EXE_ARGS}
			WORKING_DIR ${CGVARG__WORKING_DIR}
		)
	endif()

	# write to target variable
	set(${LAUNCH_JSON_CONFIG_VAR} ${JSON_LIST_STRING} PARENT_SCOPE)
endfunction()

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
	set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_ENVIRONMENT "CGV_DIR=\"${CGV_DIR}\";CGV_OPTIONS=\"${CGV_OPTIONS}\"")
	set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_ARGUMENTS "${CGVARG__ARGUMENTS}")
	set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_ENVIRONMENT "CGV_DIR=\"${CGV_DIR}\";CGV_OPTIONS=\"${CGV_OPTIONS}\"")
endfunction()

function(set_plugin_execution_working_dir target_name path)
	set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${path}")
	set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_WORKING_DIRECTORY "${path}")
endfunction()
