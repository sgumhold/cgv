
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

function(add_vscode_launch_json_content LAUNCH_JSON_CONFIG_VAR TARGET_NAME)
	cmake_parse_arguments(
		PARSE_ARGV 2 CGVARG_ "" "WORKING_DIR" "PLUGIN_ARGS;EXE_ARGS"
	)

	# determine target names
	cgv_get_static_or_exe_name(NAME_STATIC NAME_EXE ${TARGET_NAME} TRUE)

	# compose JSON list for both plugin and executable variants
	# - plugin build, standard VS Code C++ debugging
	set(CONTENT_LOCAL
"		{
			\"name\": \"Debug '${TARGET_NAME}' (cppdbg)\",
			\"type\": \"cppdbg\",
			\"request\": \"launch\",
			\"program\": \"$<TARGET_FILE:cgv_viewer>\"")
	format_vscode_launch_json_args(CONTENT_LOCAL ARGUMENT_LIST ${CGVARG__PLUGIN_ARGS})
	set(CONTENT_LOCAL "${CONTENT_LOCAL},
			\"cwd\": \"${CGVARG__WORKING_DIR}\",
			\"MIMode\": \"gdb\",
			\"setupCommands\": [
			{\n
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
		}"
	)
	# - plugin build, CodeLLDB debugging
	set(CONTENT_LOCAL "${CONTENT_LOCAL},
		{
			\"name\": \"Debug '${TARGET_NAME}' (CodeLLDB)\",
			\"type\": \"lldb\",
			\"request\": \"launch\",
			\"program\": \"$<TARGET_FILE:cgv_viewer>\"")
	format_vscode_launch_json_args(CONTENT_LOCAL ARGUMENT_LIST ${CGVARG__PLUGIN_ARGS})
	set(CONTENT_LOCAL "${CONTENT_LOCAL},
			\"cwd\": \"${CGVARG__WORKING_DIR}\"
		}"
	)
	# - single executable build, standard VS Code C++ debugging
	set(CONTENT_LOCAL "${CONTENT_LOCAL},
		{
			\"name\": \"Debug '${NAME_EXE}' (cppdbg)\",
			\"type\": \"cppdbg\",
			\"request\": \"launch\",
			\"program\": \"$<TARGET_FILE:${NAME_EXE}>\"")
	format_vscode_launch_json_args(CONTENT_LOCAL ARGUMENT_LIST ${CGVARG__EXE_ARGS})
	set(CONTENT_LOCAL "${CONTENT_LOCAL},
			\"cwd\": \"${CGVARG__WORKING_DIR}\",
			\"MIMode\": \"gdb\",
			\"setupCommands\": [
			{\n
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
		}"
	)
	# - single executable build, CodeLLDB debugging
	set(CONTENT_LOCAL "${CONTENT_LOCAL},
		{
			\"name\": \"Debug '${NAME_EXE}' (CodeLLDB)\",
			\"type\": \"lldb\",
			\"request\": \"launch\",
			\"program\": \"$<TARGET_FILE:${NAME_EXE}>\"")
	format_vscode_launch_json_args(CONTENT_LOCAL ARGUMENT_LIST ${CGVARG__EXE_ARGS})
	set(CONTENT_LOCAL "${CONTENT_LOCAL},
			\"cwd\": \"${CGVARG__WORKING_DIR}\"
		}"
	)

	# write to target variable
	set(${LAUNCH_JSON_CONFIG_VAR} ${CONTENT_LOCAL} PARENT_SCOPE)
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
	set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_ARGUMENTS "${CGVARG__ARGUMENTS}")
endfunction()

function(set_plugin_execution_working_dir target_name path)
	set_target_properties(${target_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${path}")
	set_target_properties(${target_name} PROPERTIES XCODE_SCHEME_WORKING_DIRECTORY "${path}")
endfunction()
