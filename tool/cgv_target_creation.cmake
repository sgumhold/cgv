
# helper function for lists: turns a list into a string with the ';' element seperation token replaced by the special
# semicolon token provided by the generator expression '$<SEMICOLON>'. The resulting string can ONLY be used at
# generation time (this includes instantiation of custom commands)!
function(cgv_stringify_generatortime_list OUTPUT_VAR LIST_VAR)
	set(LIST_STRING "")
	set(LIST_CONTROL_HELPER TRUE) # <-- for distinguishing the first iteration within a foreach()
	foreach(ELEM IS_FIRST IN ZIP_LISTS LIST_VAR LIST_CONTROL_HELPER)
		if (IS_FIRST)
			set(LIST_STRING "${ELEM}")
		else()
			set(LIST_STRING "${LIST_STRING}$<SEMICOLON>${ELEM}")
		endif()
	endforeach()
	set(${OUTPUT_VAR} ${LIST_STRING} PARENT_SCOPE)
endfunction()

# helper function for lists: formats a string representing the list contents that will say "<none>" if empty
function(cgv_format_list OUTPUT_VAR LIST_VAR)
	set(${OUTPUT_VAR} "<none>" PARENT_SCOPE)
	if (LIST_VAR)
		set(${OUTPUT_VAR} "${LIST_VAR}" PARENT_SCOPE)
	endif()
endfunction()

# Checks whether the given CGV-Option is set
function(cgv_has_option OUTPUT_VAR OPTION_NAME)
	foreach(CGV_OPTION ${CGV_OPTIONS})
		if (CGV_OPTION STREQUAL "${OPTION_NAME}")
			set(${OUTPUT_VAR} TRUE PARENT_SCOPE)
			return()
		endif()
	endforeach()
	set(${OUTPUT_VAR} FALSE PARENT_SCOPE)
endfunction()

# retrieves a CGV-specific property and returns its content if any, otherwise returns something that evaluates to FALSE
# under CMake's rules
function(cgv_query_property OUTPUT_VAR TARGET_NAME PROPERTY_NAME)
	if (NOT TARGET_NAME STREQUAL "GLOBAL")
		get_target_property(PROPVAL ${TARGET_NAME} ${PROPERTY_NAME})
	else()
		get_property(PROPVAL GLOBAL PROPERTY ${PROPERTY_NAME})
	endif()
	if (PROPVAL AND NOT PROPVAL STREQUAL "PROPVAL-NOTFOUND")
		set(${OUTPUT_VAR} "${PROPVAL}" PARENT_SCOPE)
	else()
		set(${OUTPUT_VAR} FALSE PARENT_SCOPE)
	endif()
endfunction()

# retrieves a CGV-specific property that is suppposed to be a list and returns its content if any, otherwise returns
# something that evaluates to and empty list under CMake's rules
function(cgv_query_listproperty OUTPUT_VAR TARGET_NAME PROPERTY_NAME)
	if (NOT TARGET_NAME STREQUAL "GLOBAL")
		get_target_property(PROPVAL ${TARGET_NAME} ${PROPERTY_NAME})
	else()
		get_property(PROPVAL GLOBAL PROPERTY ${PROPERTY_NAME})
	endif()
	if (PROPVAL AND NOT PROPVAL STREQUAL "PROPVAL-NOTFOUND")
		set(${OUTPUT_VAR} "${PROPVAL}" PARENT_SCOPE)
	else()
		set(${OUTPUT_VAR} "" PARENT_SCOPE)
	endif()
endfunction()

# helper function for properties: formats a string representing the property value that will say "<none>" if the property
# is empty
function(cgv_format_property_value OUTPUT_VAR TARGET_NAME PROPERTY_NAME)
	cgv_query_property(PROPVAL ${TARGET_NAME} ${PROPERTY_NAME})
	cgv_format_list(PROPVAL_FORMATTED "${PROPVAL}")
	set(${OUTPUT_VAR} "${PROPVAL_FORMATTED}" PARENT_SCOPE)
endfunction()

# gathers all direct dependencies of a target from all related properties
function(cgv_list_dependencies DEPS_LIST_OUT TARGET_NAME)
	# check each property storing dependencies in order
	# (TODO: may contain generator expressions - we currently ignore those)
	get_target_property(DEPS ${TARGET_NAME} LINK_LIBRARIES)
	if (DEPS STREQUAL "DEPS-NOTFOUND")
		set(DEPS "")
	endif()
	get_target_property(IFACE_LINK_LIBS ${TARGET_NAME} INTERFACE_LINK_LIBRARIES)
	if (IFACE_LINK_LIBS STREQUAL "IFACE_LINK_LIBS-NOTFOUND")
	else()
		list(APPEND DEPS "${IFACE_LINK_LIBS}")
		list(REMOVE_DUPLICATES DEPS)
	endif()
	get_target_property(GENERIC_DEPS ${TARGET_NAME} MANUALLY_ADDED_DEPENDENCIES)
	if (GENERIC_DEPS STREQUAL "GENERIC_DEPS-NOTFOUND")
	else()
		list(APPEND DEPS "${GENERIC_DEPS}")
	endif()
	list(REMOVE_DUPLICATES DEPS) # <-- this is intentionally not inside the above else()-clause

	# return result
	set(${DEPS_LIST_OUT} "${DEPS}" PARENT_SCOPE)
endfunction()

# lists all targets defined in the given directory, optionally including subdirectories
function(cgv_get_all_directory_targets TARGETS_VAR DIRECTORY)
	cmake_parse_arguments(PARSE_ARGV 2 CGVARG_ "RECURSIVE" "" "")

	set(MY_TARGETS "")
	if (CGVARG__RECURSIVE)
		get_property(SUBDIRS DIRECTORY ${DIRECTORY} PROPERTY SUBDIRECTORIES)
		foreach(SUBDIR ${SUBDIRS})
			cgv_get_all_directory_targets(SUBDIR_TARGETS ${SUBDIR} RECURSIVE)
			list(APPEND MY_TARGETS ${SUBDIR_TARGETS})
		endforeach()
	endif()

	get_property(DIR_TARGETS DIRECTORY ${DIRECTORY} PROPERTY BUILDSYSTEM_TARGETS)
	list(APPEND MY_TARGETS ${DIR_TARGETS})
	set(${TARGETS_VAR} ${MY_TARGETS} PARENT_SCOPE)
endfunction()

# checks if the given target is some kind of CGV Framework component, and if yes, optionally returns the type of the
# component
function(cgv_is_cgvtarget CHECK_RESULT_OUT TARGET_NAME)
	cmake_parse_arguments(
		PARSE_ARGV 2 CGVARG_ "" "GET_TYPE" ""
	)

	if (TARGET ${TARGET_NAME})
		cgv_query_property(TARGET_TYPE ${TARGET_NAME} CGVPROP_TYPE)
		if (TARGET_TYPE)
			set(${CHECK_RESULT_OUT} TRUE PARENT_SCOPE)
			if (CGVARG__GET_TYPE AND NOT CGVARG__GET_TYPE STREQUAL "-NOTFOUND")
				set(${CGVARG__GET_TYPE} ${TARGET_TYPE} PARENT_SCOPE)
			endif()
		else()
			set(${CHECK_RESULT_OUT} FALSE PARENT_SCOPE)
		endif()
	else()
		set(${CHECK_RESULT_OUT} FALSE PARENT_SCOPE)
	endif()
endfunction()

# recursively gathers transitive dependencies on CGV Framework components for the given target
# NOTE: when CGV_TARGETS_ONLY flag is not set, this will return UTILITY and other kind of unlinkable targets as
#       as well when they happen to appear in the dependency tree. TODO: consider filtering them out by default
function(cgv_gather_dependencies DEPS_LIST_OUT TARGET_NAME PROCESSED_DEPS)
	cmake_parse_arguments(PARSE_ARGV 3 CGVARG_ "CGV_TARGETS_ONLY" "" "")

	# when gathering only cgv dependencies, break early if TARGET_NAME is not a CGV Framework target
	# and thus won't provide any information we're interested in
	cgv_is_cgvtarget(IS_CGV_TARGET ${TARGET_NAME})
	if (NOT TARGET ${TARGET_NAME} OR (CGVARG__CGV_TARGETS_ONLY AND NOT IS_CGV_TARGET))
		return()
	endif()

	# make sure already-processed dependencies can be propagated, and break the recursion
	# cycle for this target by adding it to the list
	set(DEPS_LIST_LOCAL "${${DEPS_LIST_OUT}}")
	set(PROCESSED_DEPS_LOCAL "${${PROCESSED_DEPS}}")
	list(APPEND PROCESSED_DEPS_LOCAL ${TARGET_NAME})

	# gather direct dependencies
	cgv_list_dependencies(DEPS ${TARGET_NAME})

	# recursive expansion
	# - prepare cgv-only flag
	if (CGVARG__CGV_TARGETS_ONLY)
		set(CGV_TARGETS_ONLY_FLAG CGV_TARGETS_ONLY)
	endif()
	# - recurse for every direct dependency
	foreach (DEPENDENCY ${DEPS})
		cgv_is_cgvtarget(IS_CGV_TARGET ${DEPENDENCY})
		if (    (TARGET ${DEPENDENCY} AND NOT ${DEPENDENCY} IN_LIST PROCESSED_DEPS_LOCAL)
		    AND (NOT CGVARG__CGV_TARGETS_ONLY OR IS_CGV_TARGET))
			list(APPEND DEPS_LIST_LOCAL ${DEPENDENCY})
			cgv_gather_dependencies(DEPS_LIST_LOCAL ${DEPENDENCY} PROCESSED_DEPS_LOCAL ${CGV_TARGETS_ONLY_FLAG})
		endif()
	endforeach()
	list(REMOVE_DUPLICATES DEPS_LIST_LOCAL)

	# propagate results upwards
	set(${DEPS_LIST_OUT} "${DEPS_LIST_LOCAL}" PARENT_SCOPE)
	set(${PROCESSED_DEPS} "${PROCESSED_DEPS_LOCAL}" PARENT_SCOPE)
endfunction()

# filters all plugins from a list of targets
function(cgv_filter_for_plugins PLUGIN_LIST_OUT GUI_PROVIDER_PLUGIN_OUT TARGET_NAMES)
	# Make sure the variable requested to contain the GUI provider will evaluate to the FALSE value when no GUI provider
	# is among the given list of dependencies
	set(GUI_PROVIDER_PLUGIN_LOCAL FALSE)

	# check TYPE property of each target
	foreach (TARGET_NAME ${TARGET_NAMES})
		get_target_property(TARGET_TYPE ${TARGET_NAME} CGVPROP_TYPE)
		if (TARGET_TYPE STREQUAL "plugin")
			# TODO: hard-coded to cg_fltk for now, more flexible and robust strategies are possible
			if (TARGET_NAME STREQUAL "cg_fltk")
				set(GUI_PROVIDER_PLUGIN_LOCAL "${TARGET_NAME}")
			else()
				list(APPEND PLUGIN_LIST_LOCAL ${TARGET_NAME})
			endif()
		endif()
	endforeach()

	# return filtered list
	set(${PLUGIN_LIST_OUT} "${PLUGIN_LIST_LOCAL}" PARENT_SCOPE)
	set(${GUI_PROVIDER_PLUGIN_OUT} "${GUI_PROVIDER_PLUGIN_LOCAL}" PARENT_SCOPE)
endfunction()

# internal helper function returning the name of the static/executable counterpart of a target
function(cgv_get_static_or_exe_name STATIC_NAME_OUT EXE_NAME_OUT TARGET_NAME IS_PLUGIN)
	if (IS_PLUGIN)
		set(${EXE_NAME_OUT} "${TARGET_NAME}_exe" PARENT_SCOPE)
	endif()
	set(${STATIC_NAME_OUT} "${TARGET_NAME}_static" PARENT_SCOPE)
endfunction()

# internal helper function that takes over deferred computations that require other targets to have already been fully
# defined
# - global state the function can modify
set(VSCODE_LAUNCH_JSON_CONFIG_LIST "")
# - the actual function
function(cgv_do_deferred_ops TARGET_NAME CONFIGURING_CGV)
	# output notification of deferred operation
	get_target_property(TARGET_TYPE ${TARGET_NAME} CGVPROP_TYPE)
	message(STATUS "Performing deferred operations for ${TARGET_TYPE} '${TARGET_NAME}'")

	# do plugin-specific deferred operations
	if (TARGET_TYPE MATCHES "plugin$")
		# (1) gather all cgv dependencies and compile shaderpath
		cgv_gather_dependencies(DEPENDENCIES ${TARGET_NAME} RECURSION_CONVERGENCE_HELPER CGV_TARGETS_ONLY)
		set(SHADER_PATHS "")
		set(SHADER_PATHS_NOSC "")
		foreach(DEPENDENCY ${DEPENDENCIES})
			cgv_query_property(DEP_SHADER_PATH ${DEPENDENCY} CGVPROP_SHADERPATH)
			if (DEP_SHADER_PATH)
				list(APPEND SHADER_PATHS "${DEP_SHADER_PATH}")
			endif()
		endforeach()
		cgv_query_property(MY_SHADER_PATH ${TARGET_NAME} CGVPROP_SHADERPATH)
		if (MY_SHADER_PATH)
			list(APPEND SHADER_PATHS "${MY_SHADER_PATH}")
		endif()
		if (NOT SHADER_PATHS STREQUAL "")
			list(REMOVE_DUPLICATES SHADER_PATHS)
			set(LIST_CONTROL_HELPER TRUE) # <-- for distinguishing the first iteration within a foreach()
			foreach(SHADER_PATH IS_FIRST IN ZIP_LISTS SHADER_PATHS LIST_CONTROL_HELPER)
				if (IS_FIRST)
					set(SHADER_PATHS_NOSC "${SHADER_PATH}")
				else()
					set(SHADER_PATHS_NOSC "${SHADER_PATHS_NOSC}\\\;${SHADER_PATH}")
				endif()
			endforeach()
		else()
			set(SHADER_PATHS FALSE)
		endif()

		# (2) if the plugin is single executable-enabled, link in all static libraries - this is done in the deferred
		#     step to catch all transitively included object libraries, which CMake stupidly doesn't do by itself
		cgv_query_property(NO_EXECUTABLE ${TARGET_NAME} CGVPROP_NO_EXECUTABLE)
		if (NOT NO_EXECUTABLE)
			set(NAME_EXE ${TARGET_NAME}_exe)
			if (NOT MSVC)
				target_link_options(${NAME_EXE} PRIVATE -Wl,--copy-dt-needed-entries)
			endif()
			foreach (DEPENDENCY ${DEPENDENCIES})
				# special handling for the viewer, as the static build variants of the viewer app is called differently
				if (DEPENDENCY STREQUAL "cgv_viewer")
					target_link_libraries(${NAME_EXE} PRIVATE cgv_viewer_main)
				else()
					# for all other dependencies, we check if it is a CGV component and act appropriately
					cgv_is_cgvtarget(IS_CGV_TARGET ${DEPENDENCY} GET_TYPE DEPENDENCY_TYPE)
					if (NOT IS_CGV_TARGET)
						# this branch should trigger for all non-CGV link dependencies
						target_link_libraries(${NAME_EXE} PRIVATE ${DEPENDENCY})
					elseif (NOT DEPENDENCY_TYPE STREQUAL "app")
						# this branch should trigger for all CGV libraries and plugins
						target_link_libraries(${NAME_EXE} PRIVATE ${DEPENDENCY}_static)
					else()
						# this branch should currently trigger only if it's a CGV app
						# ...nothing to do here!
					endif()
				endif()
			endforeach()
		endif()

		# (3) extract list of plugins to load
		cgv_filter_for_plugins(REQUESTED_PLUGINS GUI_PROVIDER_PLUGIN "${DEPENDENCIES}")

		# (4) compile command line arguments
		# --- if we have a GUI provider, it will be loaded first, followed by the shader path
		if (GUI_PROVIDER_PLUGIN)
			set(CMD_LINE_ARGS_STRING "plugin:${GUI_PROVIDER_PLUGIN}")
			list(APPEND AUTOGEN_CMD_LINE_ARGS ${CMD_LINE_ARGS_STRING})
			if (SHADER_PATHS)
				set(CMD_LINE_ARGS_STRING "${CMD_LINE_ARGS_STRING} \"type(shader_config):shader_path='${SHADER_PATHS}'\"")
				list(APPEND AUTOGEN_CMD_LINE_ARGS "type(shader_config):shader_path='${SHADER_PATHS_NOSC}'")
			endif()
		endif()
		# --- append remaining plugins
		foreach(PLUGIN ${REQUESTED_PLUGINS})
			set(NEW_CMD_LINE_ARG "plugin:${PLUGIN}")
			set(CMD_LINE_ARGS_STRING "${CMD_LINE_ARGS_STRING} ${NEW_CMD_LINE_ARG}")
			list(APPEND AUTOGEN_CMD_LINE_ARGS ${NEW_CMD_LINE_ARG})
		endforeach()
		set(NEW_CMD_LINE_ARG "plugin:${TARGET_NAME}")
		set(CMD_LINE_ARGS_STRING "${CMD_LINE_ARGS_STRING} ${NEW_CMD_LINE_ARG}")
		list(APPEND AUTOGEN_CMD_LINE_ARGS ${NEW_CMD_LINE_ARG})
		# --- append custom args (if any)
		cgv_query_property(ADDITIONAL_ARGS ${TARGET_NAME} CGVPROP_ADDITIONAL_CMDLINE_ARGS)
		if (ADDITIONAL_ARGS)
			foreach(ARG ${ADDITIONAL_ARGS})
				set(ADDITIONAL_ARGS_STRING "${ADDITIONAL_ARGS_STRING} ${ARG}")
			endforeach()
			set(CMD_LINE_ARGS_STRING "${CMD_LINE_ARGS_STRING} ${ADDITIONAL_ARGS_STRING}")
		endif()

		# create launch scripts and .vscode configs
		# - check if a specific working directory was requested
		cgv_query_property(WORKING_DIR ${TARGET_NAME} CGVPROP_WORKING_DIR)
		if (NOT WORKING_DIR)
			set(WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR})
		endif()
		# - create actual launch/debug config
		set(DO_CREATE_LAUNCH_CONFIG TRUE)
		if (CONFIGURING_CGV AND NO_EXECUTABLE)
			set(DO_CREATE_LAUNCH_CONFIG FALSE)
		endif()
		if (DO_CREATE_LAUNCH_CONFIG) # <-- removed requirement that CMAKE_GENERATOR MATCHES "Make" or CMAKE_GENERATOR MATCHES "^Ninja"
			# Preamble
			set(NO_EXE_FLAG "")
			if (NO_EXECUTABLE)
				set(NO_EXE_FLAG "NO_EXECUTABLE")
			endif()
			cgv_query_property(INVOCATION_PROXY ${TARGET_NAME} CGVPROP_INVOCATION_PROXY)

			# (1) Shell script
			configure_file(
				"${CGV_DIR}/make/cmake/run_plugin.sh.in" "${CMAKE_BINARY_DIR}/run_${TARGET_NAME}.sh"
				FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
				@ONLY
			)
			file(
				GENERATE OUTPUT "${CMAKE_BINARY_DIR}/run_${TARGET_NAME}$<$<CONFIG:Debug,RelWithDebInfo,MinSizeRel>:-$<CONFIG>>.sh"
				INPUT "${CMAKE_BINARY_DIR}/run_${TARGET_NAME}.sh" TARGET ${TARGET_NAME}
				USE_SOURCE_PERMISSIONS
			)

			# (2) JetBrains IDEs (IDEA, CLion etc.)
			#   Multi-config generators are not properly supported in these IDEs (also unneccesary). Generating launch configs for them
			#   becomes problematic with multi-config generators due to the way CMake invokes file generation, so the easiest workaround
			#   is to just not generate launch configs when a multi-config generator is used.
			#   TODO: proper handling of this is possible and should be implemented at some point
			if (NOT CGV_USING_MULTI_CONFIG)
				create_idea_run_entry(
					${TARGET_NAME} ${NO_EXE_FLAG} WORKING_DIR ${WORKING_DIR}
					PLUGIN_ARGS ${AUTOGEN_CMD_LINE_ARGS};${ADDITIONAL_ARGS} EXE_ARGS ${ADDITIONAL_ARGS}
					INVOCATION_PROXY ${INVOCATION_PROXY}
				)
			endif()

			# (3) Visual Studio Code
			concat_vscode_launch_json_content(
				VSCODE_TARGET_LAUNCH_JSON_CONFIGS ${TARGET_NAME} ${NO_EXE_FLAG} WORKING_DIR ${WORKING_DIR}
				PLUGIN_ARGS ${AUTOGEN_CMD_LINE_ARGS};${ADDITIONAL_ARGS} EXE_ARGS ${ADDITIONAL_ARGS}
				INVOCATION_PROXY ${INVOCATION_PROXY}
			)
			if (NOT VSCODE_LAUNCH_JSON_CONFIG_LIST OR VSCODE_LAUNCH_JSON_CONFIG_LIST STREQUAL "")
				set(VSCODE_LAUNCH_JSON_CONFIG_LIST "${VSCODE_TARGET_LAUNCH_JSON_CONFIGS}" PARENT_SCOPE)
			elseif (CONFIGURING_CGV)
				set(VSCODE_LAUNCH_JSON_CONFIG_LIST "${VSCODE_LAUNCH_JSON_CONFIG_LIST},\n${VSCODE_TARGET_LAUNCH_JSON_CONFIGS}" PARENT_SCOPE)
			else()
				set(VSCODE_LAUNCH_JSON_CONFIG_LIST "${VSCODE_TARGET_LAUNCH_JSON_CONFIGS},\n${VSCODE_LAUNCH_JSON_CONFIG_LIST}" PARENT_SCOPE)
			endif()
		# elseif(DO_CREATE_LAUNCH_CONFIG)  <-- merged with above branch (see comment above)
			# try to set relevant options for all other generators in the hopes of ending up with a valid launch/debug
			# configuration
			cgv_get_static_or_exe_name(NAME_STATIC NAME_EXE ${TARGET_NAME} TRUE)
			set_plugin_execution_params(${TARGET_NAME} ARGUMENTS ${CMD_LINE_ARGS_STRING})
			set_plugin_execution_working_dir(${TARGET_NAME} ${WORKING_DIR})
			if (NOT NO_EXECUTABLE)
				set_plugin_execution_params(${NAME_EXE} ARGUMENTS ${ADDITIONAL_ARGS_STRING} ALTERNATIVE_COMMAND $<TARGET_FILE:${NAME_EXE}>)
				set_plugin_execution_working_dir(${NAME_EXE} ${CMAKE_CURRENT_SOURCE_DIR})
			endif()
		endif()
	endif()
endfunction()

# internal helper function that will perform final deferred operations after the very last cgv target has been
# added either by the Framework itself or by any other projects that use the Framework from the outside
# - the actual function
function(cgv_do_final_operations)
	message(STATUS "Performing final operations")

	# prelude
	get_property(IS_MULTICONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

	# prune unused CGV targets if requested
	if (CGV_EXCLUDE_UNUSED_TARGETS)
		cgv_get_all_directory_targets(CGV_FRAMEWORK_TARGETS ${CGV_DIR} RECURSIVE)
		set(ALL_USER_DEPENDENCIES "")
		cgv_query_listproperty(USER_TARGETS GLOBAL "CGVPROP_USER_TARGETS")
		foreach(USER_TARGET ${USER_TARGETS})
			cgv_gather_dependencies(DEPENDENCIES ${USER_TARGET} RECURSION_CONVERGENCE_HELPER)
			list(APPEND ALL_USER_DEPENDENCIES ${DEPENDENCIES})
		endforeach()
		list(REMOVE_DUPLICATES ALL_USER_DEPENDENCIES)
		# TODO: populate the list of build tools automatically when transitioning EVERYTHING to the new system
		set(CGV_BUILD_TOOLS "ppp;shader_test;res_prep")
		set(EXCLUDED_CGV_FRAMEWORK_TARGETS "")
		foreach(CGV_FRAMEWORK_TARGET ${CGV_FRAMEWORK_TARGETS})
			if (    NOT ${CGV_FRAMEWORK_TARGET} IN_LIST CGV_BUILD_TOOLS
				AND NOT ${CGV_FRAMEWORK_TARGET} IN_LIST ALL_USER_DEPENDENCIES)
				list(APPEND EXCLUDED_CGV_FRAMEWORK_TARGETS ${CGV_FRAMEWORK_TARGET})
				set_target_properties(${CGV_FRAMEWORK_TARGET} PROPERTIES EXCLUDE_FROM_ALL TRUE)
				set_target_properties(${CGV_FRAMEWORK_TARGET} PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE)
			endif()
		endforeach()
		message("Excluding the following unused CGV Framework targets:")
		message("- ${EXCLUDED_CGV_FRAMEWORK_TARGETS}")
	endif()

	# generate VS Code launch.json
	if (VSCODE_LAUNCH_JSON_CONFIG_LIST AND NOT VSCODE_LAUNCH_JSON_CONFIG_LIST STREQUAL "")
		configure_file("${CGV_DIR}/make/cmake/launch.json.in" "${CMAKE_SOURCE_DIR}/.vscode/launch.json" @ONLY)
		if (IS_MULTICONFIG)
			set(VSCODE_LAUNCH_JSON_CONDITION $<CONFIG:Debug>)
		else()
			set(VSCODE_LAUNCH_JSON_CONDITION  $<BOOL:TRUE>)
		endif()
		file(
			GENERATE OUTPUT "${CMAKE_SOURCE_DIR}/.vscode/launch.json"
			INPUT "${CMAKE_SOURCE_DIR}/.vscode/launch.json"
			USE_SOURCE_PERMISSIONS CONDITION ${VSCODE_LAUNCH_JSON_CONDITION}
		)
	endif()
endfunction()

# set the platform-specific link-time optimization compiler and linker flags for the given CMake target
function(cgv_set_ltoflags TARGET_NAME)
	if (MSVC)
		set(LTO_CL_FLAGS $<$<CONFIG:Release,MinSizeRel>:/GL>)
		set(LTO_LD_FLAGS $<$<CONFIG:Release,MinSizeRel>:/LTCG>)
		target_compile_options(${TARGET_NAME} PUBLIC ${LTO_CL_FLAGS})
		target_link_options(${TARGET_NAME} PUBLIC ${LTO_LD_FLAGS})
	else()
		set(LTO_CL_FLAGS $<$<CONFIG:Release,MinSizeRel>:-flto$<$<CXX_COMPILER_ID:GNU>:=auto> $<$<CXX_COMPILER_ID:Clang>:-fwhole-program-vtables>>)
		set(LTO_LD_FLAGS $<$<CONFIG:Release,MinSizeRel>:-flto$<$<CXX_COMPILER_ID:GNU>:=auto> $<$<CXX_COMPILER_ID:Clang>:-fwhole-program-vtables>>)
		get_target_property(TARGET_TYPE ${TARGET_NAME} TYPE)
		if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND TARGET_TYPE STREQUAL "EXECUTABLE")
			set(LTO_LD_FLAGS ${LTO_LD_FLAGS} $<$<CONFIG:Release,MinSizeRel>:-fwhole-program>)
		endif()
		target_compile_options(${TARGET_NAME} PUBLIC ${LTO_CL_FLAGS})
		target_link_options(${TARGET_NAME} PUBLIC ${LTO_LD_FLAGS})
	endif()
endfunction()

# enable link-time optimization for the given CGV component
function(cgv_enable_lto TARGET_NAME)
	cgv_set_ltoflags(${TARGET_NAME})
	cgv_is_cgvtarget(IS_CGV_TARGET ${TARGET_NAME} GET_TYPE TARGET_TYPE)
	if (IS_CGV_TARGET)
		set(IS_PLUGIN FALSE)
		if (TARGET_TYPE MATCHES "plugin$")
			set(IS_PLUGIN TRUE)
		endif()
		cgv_get_static_or_exe_name(NAME_STATIC NAME_EXE ${TARGET_NAME} ${IS_PLUGIN})
		cgv_set_ltoflags(${NAME_STATIC})
		if (IS_PLUGIN)
			cgv_query_property(NO_EXECUTABLE ${TARGET_NAME} "CGVPROP_NO_EXECUTABLE")
			if (NOT NO_EXECUTABLE)
				cgv_set_ltoflags(${NAME_EXE})
			endif()
		endif()
	endif()
endfunction()

# add a target to the build system that is decorated with several CGV Framework-specific custom properties, allowing for
# advanced features like automatic launch/debug command line generation, auto-configured single executable builds etc.
function(cgv_add_target NAME)
	cmake_parse_arguments(
		PARSE_ARGV 1 CGVARG_
		"NO_EXECUTABLE" "TYPE;OVERRIDE_SHARED_EXPORT_DEFINE;OVERRIDE_FORCE_STATIC_DEFINE;WORKING_DIR"
		"SOURCES;PPP_SOURCES;HEADERS;RESOURCES;AUDIO_RESOURCES;SHADER_SOURCES;ADDITIONAL_PRIVATE_DEFINES;ADDITIONAL_PUBLIC_DEFINES;DEPENDENCIES;LINKTIME_PLUGIN_DEPENDENCIES;ADDITIONAL_INCLUDE_PATHS;ADDITIONAL_LINKER_PATHS;ADDITIONAL_CMDLINE_ARGS;INVOCATION_PROXY"
	)

	# prelude
	set(IS_LIBRARY FALSE)
	set(IS_CORELIB FALSE)
	set(IS_PLUGIN FALSE)
	set(IS_STATIC FALSE)
	if (NOT CGVARG__TYPE)
		message(FATAL_ERROR "cgv_add_target(): no type specified for CGV build target '${NAME}'!")
	elseif (CGVARG__TYPE STREQUAL "corelib")
		set(IS_LIBRARY TRUE)
		set(IS_CORELIB TRUE)
		string(SUBSTRING ${NAME} 4 -1 HEADER_INSTALL_DIR)
		set(HEADER_INSTALL_DIR ${CGV_INCLUDE_DEST}/${HEADER_INSTALL_DIR})
		set(EXPORT_TARGET cgv)
	else()
		if (CGVARG__TYPE MATCHES "library$")
			set(IS_LIBRARY TRUE)
		elseif (CGVARG__TYPE MATCHES "plugin$")
			set(IS_PLUGIN TRUE)
		endif()
		if (CGVARG__TYPE MATCHES "^static")
			set(IS_STATIC TRUE)
		endif()
		set(HEADER_INSTALL_DIR ${CGV_LIBS_INCLUDE_DEST}/${NAME})
		set(EXPORT_TARGET cgv_libs)
	endif()

	# process collected files to be attached to the target (and attach the processing results if any)
	# - PPP generated sources/headers/generic files
	if (CGVARG__PPP_SOURCES)
		ppp_compile("${CGV_DIR}"
				PPP_FILES
				PPP_INCLUDES
				PPP_INSTALL_DIR
				${CGVARG__PPP_SOURCES})
		install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
	endif()
	# - shader files
	set(SHADER_PATH "")
	if (CGVARG__SHADER_SOURCES)
		# compile shaderpath for this plugin: walk through every shader source to collect the absolute
		# path to their parent directories
		foreach (SHADER_SOURCE ${CGVARG__SHADER_SOURCES})
			cmake_path(
				ABSOLUTE_PATH SHADER_SOURCE BASE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} NORMALIZE
				OUTPUT_VARIABLE SHADER_SOURCE_ABSOLUTE_PATH)
			cmake_path(GET SHADER_SOURCE_ABSOLUTE_PATH PARENT_PATH SHADER_SOURCE_PARENT_PATH)
			list(APPEND SHADER_PATH ${SHADER_SOURCE_PARENT_PATH})
		endforeach()
		list(REMOVE_DUPLICATES SHADER_PATH)

		# perform shader test
		# TODO: Currently relies on the hardcoded default shaderpath of the framework. Move to deferred ops
		#       so the exact shaderpath computed from the transitive dependencies can be used
		shader_test(${NAME}  ST_FILES ST_INCLUDE ST_INSTALL_DIR SHADER_REG_INCLUDE_FILE ${CGVARG__SHADER_SOURCES})

		install(DIRECTORY ${ST_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
	endif ()
	# - generic resources (e.g. images, icons)
	if (CGVARG__RESOURCES)
		cgv_prepare_resources(${CMAKE_CURRENT_SOURCE_DIR} RESOURCE_SRCFILES ${CGVARG__RESOURCES})
	endif()
	# - audio resources
	if (CGVARG__AUDIO_RESOURCES AND CGV_BUILD_WITH_AUDIO)
		cgv_prepare_resources(${CMAKE_CURRENT_SOURCE_DIR} AUDIO_RESOURCE_SRCFILES ${CGVARG__AUDIO_RESOURCES})
	else()
		set(CGVARG__AUDIO_RESOURCES "")
	endif()

	# prepare all files for inclusion in the target
	set(ALL_SOURCES
		${CGVARG__SOURCES} ${PPP_FILES} ${CGVARG__PPP_SOURCES} ${CGVARG__HEADERS} ${ST_FILES} ${SHADERS}
		${RESOURCE_SRCFILES} ${CGVARG__RESOURCES}
	)
	if (CGV_BUILD_WITH_AUDIO)
		list(APPEND ALL_SOURCES ${AUDIO_RESOURCE_SRCFILES} ${CGVARG__AUDIO_RESOURCES})
	endif()

	# determine name for static variant
	cgv_get_static_or_exe_name(NAME_STATIC NAME_EXE ${NAME} ${IS_PLUGIN})
	string(TOUPPER ${NAME} NAME_UPPER) # <-- used in compile-time definitions

	# for plugin builds
	if (IS_STATIC)
		# Moreso than indicating the type of object that will be generated from the target, the _static suffix
		# indicates in what type of build the target will be used. The framework knows plugin builds and single
		# executable builds - the latter one links the static variants of all targets it depends on into an
		# executable, while the former will use the non-static versions of all library targets (usually, those
		# would be shared variants) with plugins loaded at runtime. Static libraries however do not have a
		# shared version, so the target with non-static name IS STILL A STATIC LIBRARY - it just provides the
		# necessary information for shared builds to be able to (statically) link to them.
		add_library(${NAME} STATIC ${ALL_SOURCES})
	else()
		add_library(${NAME} SHARED ${ALL_SOURCES})
	endif()

	set_target_properties(${NAME} PROPERTIES CGVPROP_TYPE "${CGVARG__TYPE}")
	set_target_properties(${NAME} PROPERTIES CGVPROP_SHADERPATH "${SHADER_PATH}")
	set_target_properties(${NAME} PROPERTIES DEFINE_SYMBOL "") # <-- disable CMake's automatic definition of "_EXPORTS" defines for shared libraries
	# set the _EXPORTS define to the given custom name if any, othewise use the default convention
	if (NOT IS_STATIC AND CGVARG__OVERRIDE_SHARED_EXPORT_DEFINE)
		target_compile_definitions(${NAME} PRIVATE "${CGVARG__OVERRIDE_SHARED_EXPORT_DEFINE}")
	else()
		target_compile_definitions(${NAME} PRIVATE "${NAME_UPPER}_EXPORTS")
	endif()
	# set custom defines
	target_compile_definitions(${NAME} PRIVATE ${CGVARG__ADDITIONAL_PRIVATE_DEFINES})
	target_compile_definitions(${NAME} PUBLIC  ${CGVARG__ADDITIONAL_PUBLIC_DEFINES})
	# set compile options
	target_compile_options(${NAME} PRIVATE ${CGV_CLANG_SPECIFIC_DEBUG_FLAGS})
	# handle dependencies
	target_link_directories(${NAME} PRIVATE ${CGVARG__ADDITIONAL_LINKER_PATHS})
	foreach (DEPENDENCY ${CGVARG__DEPENDENCIES})
		# find out dependency type
		cgv_is_cgvtarget(IS_CGV_TARGET ${DEPENDENCY} GET_TYPE DEPENDENCY_TYPE)
		if (NOT IS_CGV_TARGET OR DEPENDENCY_TYPE STREQUAL "library" OR DEPENDENCY_TYPE STREQUAL "corelib")
			# this branch should trigger whenever it's some generic, non-CGV dependency or if it's a CGV library
			target_link_libraries(${NAME} PUBLIC ${DEPENDENCY})
		else()
			# this branch should currently trigger only if it's a CGV plugin or app
			if (DEPENDENCY_TYPE STREQUAL "plugin" AND ${DEPENDENCY} IN_LIST CGVARG__LINKTIME_PLUGIN_DEPENDENCIES)
				# We need to actually link to the DLL/shared object of this plugin
				target_link_libraries(${NAME} PRIVATE ${DEPENDENCY})
			else()
				# We only need this dependency to be present (for its shaders, resources, independent functionality etc.)
				add_dependencies(${NAME} ${DEPENDENCY})
			endif()
		endif()
	endforeach()
	if (IS_PLUGIN AND CGVARG__NO_EXECUTABLE)
		set_target_properties(${NAME} PROPERTIES CGVPROP_NO_EXECUTABLE TRUE)
	endif()

	target_include_directories(${NAME} PUBLIC
			${CGVARG__ADDITIONAL_INCLUDE_PATHS}
			"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
			"$<BUILD_INTERFACE:${CGV_DIR}>"
			"$<BUILD_INTERFACE:${PPP_INCLUDES}>"
			"$<BUILD_INTERFACE:${ST_INCLUDES}>")
	if (IS_CORELIB)
		target_include_directories(${NAME} PUBLIC "$<INSTALL_INTERFACE:include>")
	else ()
		target_include_directories(${NAME} PUBLIC
				"$<BUILD_INTERFACE:${CGV_DIR}/libs>"
				"$<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>")
	endif ()

	install(TARGETS ${NAME} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})

	# for single executable builds
	set(FORCE_STATIC_DEFINE "${NAME_UPPER}_FORCE_STATIC")
	if (CGVARG__OVERRIDE_FORCE_STATIC_DEFINE)
		set(FORCE_STATIC_DEFINE "${CGVARG__OVERRIDE_FORCE_STATIC_DEFINE}")
	endif()
	set(PRIVATE_STATIC_TARGET_DEFINES "${FORCE_STATIC_DEFINE}" "REGISTER_SHADER_FILES" "${CGVARG__ADDITIONAL_PRIVATE_DEFINES}")
	add_library(${NAME_STATIC} OBJECT ${ALL_SOURCES} ${SHADER_REG_INCLUDE_FILE})
	set_target_properties(${NAME_STATIC} PROPERTIES CGVPROP_TYPE "${CGVARG__TYPE}")
	set_target_properties(${NAME_STATIC} PROPERTIES CGVPROP_SHADERPATH "${SHADER_PATH}")
	target_compile_definitions(${NAME_STATIC} PRIVATE ${PRIVATE_STATIC_TARGET_DEFINES})
	target_compile_definitions(${NAME_STATIC} PUBLIC "CGV_FORCE_STATIC" ${CGVARG__ADDITIONAL_PUBLIC_DEFINES})
	target_compile_options(${NAME_STATIC} PUBLIC ${CGV_CLANG_SPECIFIC_DEBUG_FLAGS})

	target_link_directories(${NAME_STATIC} PUBLIC ${CGVARG__ADDITIONAL_LINKER_PATHS})
	if (NOT MSVC)
		target_link_options(${NAME_STATIC} PUBLIC -Wl,--copy-dt-needed-entries)
	endif()
	foreach (DEPENDENCY ${CGVARG__DEPENDENCIES})
		# for all dependencies, we check if it is a CGV component and act appropriately
		cgv_is_cgvtarget(IS_CGV_TARGET ${DEPENDENCY} GET_TYPE DEPENDENCY_TYPE)
		if (NOT IS_CGV_TARGET)
			# this branch should trigger for all non-CGV link dependencies
			target_link_libraries(${NAME_STATIC} PUBLIC ${DEPENDENCY})
		elseif (NOT DEPENDENCY_TYPE STREQUAL "app")
			# this branch should trigger for all CGV libraries and plugins
			target_link_libraries(${NAME_STATIC} PUBLIC ${DEPENDENCY}_static)
		else()
			# this branch should currently trigger only if it's a CGV app
			# ...nothing to do here!
		endif()
	endforeach()

	target_include_directories(${NAME_STATIC} PUBLIC
		${CGVARG__ADDITIONAL_INCLUDE_PATHS}
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
		"$<BUILD_INTERFACE:${CGV_DIR}>"
		"$<BUILD_INTERFACE:${PPP_INCLUDES}>"
		"$<BUILD_INTERFACE:${ST_INCLUDE}>"
		"$<INSTALL_INTERFACE:include>")
	if (NOT IS_CORELIB)
		target_include_directories(${NAME_STATIC} PUBLIC $<BUILD_INTERFACE:${CGV_DIR}/libs>)
	endif ()

	# Prevent Clang complaining about illegal characters in string literals when baking base64-encoded shaders into the
	# single executable
	cgv_has_option(HAS_OPTION_ENCODE_SHADER_BASE64 "ENCODE_SHADER_BASE64")
	if (    HAS_OPTION_ENCODE_SHADER_BASE64
	    AND (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
		target_compile_options(${NAME_STATIC} PRIVATE -Wno-invalid-source-encoding)
	endif()

	# add single executable version if not disabled for this target
	if (IS_PLUGIN AND NOT CGVARG__NO_EXECUTABLE)
		add_executable(${NAME_EXE})
		set_target_properties(${NAME_EXE} PROPERTIES OUTPUT_NAME "${NAME}")
		target_compile_definitions(${NAME_EXE} PRIVATE ${PRIVATE_STATIC_TARGET_DEFINES})
		target_include_directories(
			${NAME_EXE} PUBLIC
			${CGVARG__ADDITIONAL_INCLUDE_PATHS} "$<BUILD_INTERFACE:${CGV_DIR}>" "$<BUILD_INTERFACE:${CGV_DIR}/libs>"
			"$<BUILD_INTERFACE:${PPP_INCLUDES}>" "$<BUILD_INTERFACE:${ST_INCLUDE}>" "$<INSTALL_INTERFACE:include>"
		)
		target_link_libraries(${NAME_EXE} PRIVATE ${NAME_STATIC})
	endif()

	# observe STDCPP17 option
	if (CGV_STDCPP17)
		target_compile_features(${NAME} PRIVATE cxx_std_17)
		target_compile_features(${NAME_STATIC} PUBLIC cxx_std_17)
		set_target_properties(${NAME} ${NAME_STATIC} PROPERTIES CXX_STANDARD 17)
		set_target_properties(${NAME} ${NAME_STATIC} PROPERTIES CXX_STANDARD_REQUIRED ON)
		if (MSVC)
			target_compile_options(${NAME} PRIVATE "/Zc:__cplusplus")
			target_compile_options(${NAME_STATIC} PUBLIC "/Zc:__cplusplus")
		endif()
	endif()

	# handle whole-program / link-time optimization
	if (CGV_LTO_ON_RELEASE)
		cgv_enable_lto(${NAME})
	endif()

	# in case of Debug config, set _DEBUG and DEBUG defines for both targets
	# (for historic reasons, CGV targets expect these instead of relying on NDEBUG)
	set(DEBUG_COMPILE_DEFS $<$<CONFIG:Debug>:_DEBUG> $<$<CONFIG:Debug>:DEBUG>)
	target_compile_definitions(${NAME} PRIVATE ${DEBUG_COMPILE_DEFS})
	target_compile_definitions(${NAME_STATIC} PRIVATE ${DEBUG_COMPILE_DEFS})

	# commit additional command line args for the subsequent generation phase
	if (IS_PLUGIN)
		set_target_properties(${NAME} PROPERTIES CGVPROP_ADDITIONAL_CMDLINE_ARGS "${CGVARG__ADDITIONAL_CMDLINE_ARGS}")
	endif()

	# commit invocation proxy program/script if requested
	if (IS_PLUGIN AND CGVARG__INVOCATION_PROXY)
		set_target_properties(${NAME} PROPERTIES CGVPROP_INVOCATION_PROXY "${CGVARG__INVOCATION_PROXY}")
	endif()

	# commit custom run/debug working directory if requested
	if (IS_PLUGIN AND CGVARG__WORKING_DIR)
		set_target_properties(${NAME} PROPERTIES CGVPROP_WORKING_DIR "${CGVARG__WORKING_DIR}")
	endif()

	# record whether this target is added by the CGV Framework itself or by another project using it
	if (NOT CGV_IS_CONFIGURING)
		list(APPEND MY_TARGETS ${NAME} ${NAME_STATIC})
		if (IS_PLUGIN AND NOT CGVARG__NO_EXECUTABLE)
			list(APPEND MY_TARGETS ${NAME_EXE})
		endif()
		set_property(GLOBAL APPEND PROPERTY "CGVPROP_USER_TARGETS" ${MY_TARGETS})
	endif()

	# IDE fluff
	# - bin each source file into appropriate IDE filter category
	source_group(Sources FILES ${CGVARG__SOURCES} ${CGVARG__HEADERS} ${SHADER_REG_INCLUDE_FILE} ${CGVARG__PPP_SOURCES})
	source_group(Sources/generated FILES ${PPP_FILES})
	source_group(Shaders FILES ${CGVARG__SHADER_SOURCES})
	source_group(Shaders/converted FILES ${ST_FILES})
	source_group(Resources FILES ${CGVARG__RESOURCES} ${CGVARG__AUDIO_RESOURCES})
	source_group(Resources/converted FILES ${RESOURCE_SRCFILES} ${AUDIO_RESOURCE_SRCFILES})
	# - assign project-level filter
	if (IS_CORELIB)
		if (IS_STATIC)
			set_target_properties(${NAME} PROPERTIES FOLDER "Core/static")
		else()
			set_target_properties(${NAME} PROPERTIES FOLDER "Core")
		endif()
	elseif (IS_LIBRARY)
		if (IS_STATIC)
			set_target_properties(${NAME} PROPERTIES FOLDER "Lib/static")
		else()
			set_target_properties(${NAME} PROPERTIES FOLDER "Lib")
		endif()
	elseif (IS_PLUGIN AND CGV_IS_CONFIGURING)
		# we only assign CGV-internal plugins to the plugins (or tests, for that matter) filter so
		# application targets from external projects that use the framework are immediatly visible
		if (CGV_CONFIGURING_TESTS)
			set_target_properties(${NAME} PROPERTIES FOLDER "Tests")
			if (NOT CGVARG__NO_EXECUTABLE)
				set_target_properties(${NAME_EXE} PROPERTIES FOLDER "Tests")
			endif()
		elseif (CGVARG__NO_EXECUTABLE)
			set_target_properties(${NAME} PROPERTIES FOLDER "Plugin")
		else()
			set_target_properties(${NAME} PROPERTIES FOLDER "Application Plugin")
			set_target_properties(${NAME_EXE} PROPERTIES FOLDER "Application Plugin")
		endif()
	endif()
	set_target_properties(${NAME_STATIC} PROPERTIES FOLDER "_obj")

	# schedule deferred ops
	# - for the created target
	if (CGV_IS_CONFIGURING)
		cmake_language(EVAL CODE "cmake_language(DEFER DIRECTORY ${CGV_DIR} CALL cgv_do_deferred_ops [[${NAME}]] TRUE)")
	else()
		cmake_language(EVAL CODE "cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL cgv_do_deferred_ops [[${NAME}]] FALSE)")
	endif()
	# - update final pass
	cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CANCEL_CALL "_999_FINALOPS")
	cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} ID "_999_FINALOPS" CALL cgv_do_final_operations)

	install(TARGETS ${NAME_STATIC} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
	if (IS_PLUGIN AND NOT CGVARG__NO_EXECUTABLE)
		install(TARGETS ${NAME_EXE} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
	endif()
endfunction()

# add sources of a custom type with an associated build rule to the given CGV Framework component
# NOTES:
#	(1) when specifying the build output filename, every occurence of <<<FN>>> will be replaced by the pure name without
#	    path and without the file extension (starting from the last dot) of the input source file, and every occurence of
#	    <<<EXT>>> will be replaced with the original file extension (without the preceding dot) of the original input
#	    source file
#	(2) when specifying the tool command arguments, every occurence of <<<INFILE>>> will be replaced with the fully
#	    qualified path to your source file, every occurence of <<<OUTFILE>>> will be replaced with the fully qualified
#	    path to your target file build from the source file, and every occurence of <<<INFILE_PATH>>> will be replaced
#	    with the fully qualified path to the directory containing the source file
#	(3) by default, the rule is added to both normal (shared) and static variants of the component - you can select to
#	    add it to only one or the other (or explicitly both) by specifying the SHARED and/or STATIC flags
#	(4) secondary files included by the sources that can not be compiled by themselves (e.g. headers of some sort) can be
#	    declared for inclusion in IDEs by listing them as HEADERS
function(cgv_add_custom_sources TARGET_NAME)
	cmake_parse_arguments(
		PARSE_ARGV 1 CGVARG_ "SHARED;STATIC" "OUTFILE_TEMPLATE;BUILD_TOOL;BUILD_SUBDIR" "SOURCES;HEADERS;BUILD_TOOL_ARGS"
	)

	# preliminaries
	cgv_get_static_or_exe_name(NAME_STATIC NAME_EXE ${TARGET_NAME} TRUE)
	string(SUBSTRING ${CGVARG__BUILD_SUBDIR} 0 1 FIRST_LETTER)
	string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
	string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER}\\1" SOURCE_GROUP_BASE "${CGVARG__BUILD_SUBDIR}")

	# loop through every source file and add it to the build system using the provided custom rule
	foreach(SOURCE ${CGVARG__SOURCES})
		# preprocess filenames
		get_filename_component(SRC_FULLPATH ${SOURCE} ABSOLUTE) # <-- evaluates with respect to CMAKE_CURRENT_SOURCE_DIR
		get_filename_component(SRC_PATH_ONLY ${SRC_FULLPATH} DIRECTORY)
		get_filename_component(SRC_FILE ${SOURCE} NAME)
		get_filename_component(SRC_FILE_WITHOUT_EXT ${SOURCE} NAME_WLE)
		get_filename_component(SRC_FILE_EXT ${SOURCE} LAST_EXT)
		string(REGEX REPLACE ".(.*)" "\\1" SRC_FILE_EXT_WITHOUT_DOT ${SRC_FILE_EXT})
		string(REGEX REPLACE "\\<\\<\\<FN>>>" "${SRC_FILE_WITHOUT_EXT}" OFILE_PROCESSED ${CGVARG__OUTFILE_TEMPLATE})
		string(REGEX REPLACE "\\<\\<\\<EXT>>>" "${SRC_FILE_EXT_WITHOUT_DOT}" OFILE ${OFILE_PROCESSED})
		set(OFILE_FULLPATH "${CMAKE_CURRENT_BINARY_DIR}/${CGVARG__BUILD_SUBDIR}/${OFILE}")

		# determine which variants of the component to add the custom source to
		if (NOT CGVARG__SHARED AND NOT CGVARG__STATIC)
			set(NO_VARIANT_FLAGS TRUE)
		endif()
		if (NO_VARIANT_FLAGS OR CGVARG__SHARED)
			set(ADD_TO_SHARED TRUE)
		endif()
		if (NO_VARIANT_FLAGS OR CGVARG__STATIC)
			set(ADD_TO_STATIC TRUE)
		endif()

		# add custom build rule for the specified sources
		set(BUILD_TOOL "$<IF:$<TARGET_EXISTS:${CGVARG__BUILD_TOOL}>,$<TARGET_FILE:${CGVARG__BUILD_TOOL}>,${CGVARG__BUILD_TOOL}>")
		# - instantiate argument templates
		set(TOOL_ARGS "")
		foreach(TOOL_ARG ${CGVARG__BUILD_TOOL_ARGS})
			string(REGEX REPLACE "\\<\\<\\<INFILE>>>" "${SRC_FULLPATH}" TOOL_ARG_PROCESSED0 ${TOOL_ARG})
			string(REGEX REPLACE "\\<\\<\\<OUTFILE>>>" "${OFILE_FULLPATH}" TOOL_ARG_PROCESSED1 ${TOOL_ARG_PROCESSED0})
			string(REGEX REPLACE "\\<\\<\\<INFILE_PATH>>>" "${SRC_PATH_ONLY}" TOOL_ARG_PROCESSED ${TOOL_ARG_PROCESSED1})
			list(APPEND TOOL_ARGS ${TOOL_ARG_PROCESSED})
		endforeach()
		# - add the actual build rule
		cgv_stringify_generatortime_list(CGV_OPTIONS_STRING "${CGV_OPTIONS}")
		add_custom_command(
			OUTPUT ${OFILE_FULLPATH}
			COMMAND ${CMAKE_COMMAND} -E env CGV_DIR="${CGV_DIR}" CGV_OPTIONS="${CGV_OPTIONS_STRING}" ${BUILD_TOOL}
			ARGS ${TOOL_ARGS}
			WORKING_DIRECTORY $<PATH:GET_PARENT_PATH,${BUILD_TOOL}>
			DEPENDS "${SRC_FULLPATH}" ${BUILD_TOOL}
		)
		# - tie the rule to the appropriate targets
		if (ADD_TO_SHARED)
			target_sources(${TARGET_NAME} PRIVATE "${OFILE_FULLPATH}" "${SRC_FULLPATH}")
		elseif (NOT (CMAKE_GENERATOR MATCHES "Make" OR CMAKE_GENERATOR MATCHES "^Ninja"))
			# in case of IDE generators we still want to display the files even if they're not going to be build
			target_sources(${TARGET_NAME} PRIVATE "${SRC_FULLPATH}")
			set_source_files_properties(
				"${SRC_FULLPATH}" TARGET_DIRECTORY ${TARGET_NAME}
				PROPERTIES HEADER_FILE_ONLY TRUE
			)
		endif()
		if (ADD_TO_STATIC)
			target_sources(${NAME_STATIC} PRIVATE "${OFILE_FULLPATH}" "${SRC_FULLPATH}")
		elseif (NOT (CMAKE_GENERATOR MATCHES "Make" OR CMAKE_GENERATOR MATCHES "^Ninja"))
			# in case of IDE generators we still want to display the files even if they're not going to be build
			target_sources(${NAME_STATIC} PRIVATE "${SRC_FULLPATH}")
			set_source_files_properties(
				"${SRC_FULLPATH}" TARGET_DIRECTORY ${NAME_STATIC}
				PROPERTIES HEADER_FILE_ONLY TRUE
			)
		endif()
		# - IDE fluff
		source_group("${SOURCE_GROUP_BASE}" FILES ${SRC_FULLPATH})
		source_group("${SOURCE_GROUP_BASE}/processed" FILES ${OFILE_FULLPATH})
	endforeach()

	# loop through every header and make them show up in IDEs
	if (NOT (CMAKE_GENERATOR MATCHES "Make" OR CMAKE_GENERATOR MATCHES "^Ninja"))
		foreach(HEADER ${CGVARG__HEADERS})
			target_sources(${TARGET_NAME} PRIVATE ${HEADER})
			target_sources(${NAME_STATIC} PRIVATE ${HEADER})
			set_source_files_properties(${HEADER} PROPERTIES HEADER_FILE_ONLY TRUE)
			source_group("${SOURCE_GROUP_BASE}" FILES ${HEADER})
		endforeach()
	endif()
endfunction()




# DEPRECATED - kept for backwards compatibility. To-be-removed!
function(cgv_create_lib NAME)
	cmake_parse_arguments(ARGS "CORE_LIB" "" "SOURCES;PPP_SOURCES;SHADER_SOURCES;DEPENDENCIES" ${ARGN})

	if (ARGS_CORE_LIB)
		string(SUBSTRING ${NAME} 4 -1 HEADER_INSTALL_DIR)
		set(HEADER_INSTALL_DIR ${CGV_INCLUDE_DEST}/${HEADER_INSTALL_DIR})

		set(EXPORT_TARGET cgv)
	else ()
		set(HEADER_INSTALL_DIR ${CGV_LIBS_INCLUDE_DEST}/${NAME})

		set(EXPORT_TARGET cgv_libs)
	endif ()

	if (ARGS_PPP_SOURCES)
		ppp_compile("${CGV_DIR}"
				PPP_FILES
				PPP_INCLUDES
				PPP_INSTALL_DIR
				${ARGS_PPP_SOURCES})

		install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
	endif ()

	set(LIB_SHADER_PATH "")
	if (ARGS_SHADER_SOURCES)
		foreach (SHADER_SOURCE ${ARGS_SHADER_SOURCES})
			cmake_path(
				ABSOLUTE_PATH SHADER_SOURCE BASE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} NORMALIZE
				OUTPUT_VARIABLE SHADER_SOURCE_ABSOLUTE_PATH)
			cmake_path(GET SHADER_SOURCE_ABSOLUTE_PATH PARENT_PATH SHADER_SOURCE_PARENT_PATH)
			list(APPEND LIB_SHADER_PATH ${SHADER_SOURCE_PARENT_PATH})
		endforeach()
		list(REMOVE_DUPLICATES LIB_SHADER_PATH)
		shader_test(${NAME}
				ST_FILES
				ST_INCLUDES
				ST_INSTALL_DIR
				SHADER_REG_INCLUDE_FILE
				${ARGS_SHADER_SOURCES})

		install(DIRECTORY ${ST_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
	endif ()

	string(TOUPPER ${NAME} NAME_UPPER)
	set(NAME_STATIC ${NAME}_static)
	set(ALL_SOURCES ${ARGS_SOURCES} ${PPP_FILES} ${ST_FILES} ${SHADERS})

	# Shared Library
	add_library(${NAME} SHARED ${ALL_SOURCES})
	if (ARGS_CORE_LIB)
		set_target_properties(${NAME} PROPERTIES CGVPROP_TYPE "corelib")
	else()
		set_target_properties(${NAME} PROPERTIES CGVPROP_TYPE "library")
	endif()
	set_target_properties(${NAME} PROPERTIES CGVPROP_SHADERPATH "${LIB_SHADER_PATH}")
	target_compile_definitions(${NAME} PRIVATE "${NAME_UPPER}_EXPORTS")
	target_compile_options(${NAME} PRIVATE ${CGV_CLANG_SPECIFIC_DEBUG_FLAGS})
	foreach (DEPENDENCY ${ARGS_DEPENDENCIES})
		target_link_libraries(${NAME} PUBLIC ${DEPENDENCY})
	endforeach ()

	target_include_directories(${NAME} PUBLIC
			$<BUILD_INTERFACE:${CGV_DIR}>
			"$<BUILD_INTERFACE:${PPP_INCLUDES}>"
			"$<BUILD_INTERFACE:${ST_INCLUDES}>")
	if (ARGS_CORE_LIB)
		target_include_directories(${NAME} PUBLIC $<INSTALL_INTERFACE:include>)
	else ()
		target_include_directories(${NAME} PUBLIC
				$<BUILD_INTERFACE:${CGV_DIR}/libs>
				$<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
	endif ()


	# Static Library
	add_library(${NAME_STATIC} OBJECT ${ALL_SOURCES} ${SHADER_REG_INCLUDE_FILE})
	target_compile_definitions(${NAME_STATIC} PUBLIC "CGV_FORCE_STATIC" "REGISTER_SHADER_FILES")
	target_compile_options(${NAME} PUBLIC ${CGV_CLANG_SPECIFIC_DEBUG_FLAGS})
	foreach (DEPENDENCY ${ARGS_DEPENDENCIES})
		if (${DEPENDENCY} STREQUAL "${CMAKE_DL_LIBS}")
			continue()
		endif ()
		target_link_libraries(${NAME_STATIC} PUBLIC ${DEPENDENCY}_static)
	endforeach ()

	target_include_directories(${NAME_STATIC} PUBLIC
			$<BUILD_INTERFACE:${CGV_DIR}>
			"$<BUILD_INTERFACE:${PPP_INCLUDES}>"
			"$<BUILD_INTERFACE:${ST_INCLUDES}>"
			$<INSTALL_INTERFACE:include>)
	if (NOT ARGS_CORE_LIB)
		target_include_directories(${NAME_STATIC} PUBLIC $<BUILD_INTERFACE:${CGV_DIR}/libs>)
	endif ()


	# handle whole-program / link-time optimization
	if (CGV_LTO_ON_RELEASE)
		cgv_enable_lto(${NAME})
	endif()

	# observe STDCPP17 option
	if (CGV_STDCPP17)
		target_compile_features(${NAME} PRIVATE cxx_std_17)
		target_compile_features(${NAME_STATIC} PUBLIC cxx_std_17)
		set_target_properties(${NAME} ${NAME_STATIC} PROPERTIES CXX_STANDARD 17)
		set_target_properties(${NAME} ${NAME_STATIC} PROPERTIES CXX_STANDARD_REQUIRED ON)
		if (MSVC)
			target_compile_options(${NAME} PRIVATE "/Zc:__cplusplus")
			target_compile_options(${NAME_STATIC} PUBLIC "/Zc:__cplusplus")
		endif()
	endif()

	# IDE fluff
	if (ARGS_CORE_LIB)
		set_target_properties(${NAME} PROPERTIES FOLDER "Core")
	else()
		set_target_properties(${NAME} PROPERTIES FOLDER "Lib")
	endif()
	set_target_properties(${NAME_STATIC} PROPERTIES FOLDER "_obj")

	install(TARGETS ${NAME} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
	install(TARGETS ${NAME_STATIC} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
endfunction()
