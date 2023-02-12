
# helper function for lists: formats a string representing the list contents that will say "<none>" if empty
function(cgv_format_list OUTPUT_VAR LIST_VAR)
	set(${OUTPUT_VAR} "<none>" PARENT_SCOPE)
	if (LIST_VAR)
		set(${OUTPUT_VAR} "${LIST_VAR}" PARENT_SCOPE)
	endif()
endfunction()

# retrieves a CGV-specific property and returns its content if any, otherwise returns something that evaluates to FALSE under
# CMake's rules
function(cgv_query_property OUTPUT_VAR TARGET_NAME PROPERTY_NAME)
	get_target_property(PROPVAL ${TARGET_NAME} ${PROPERTY_NAME})
	if (PROPVAL AND NOT PROPVAL STREQUAL "PROPVAL-NOTFOUND")
		set(${OUTPUT_VAR} "${PROPVAL}" PARENT_SCOPE)
	else()
		set(${OUTPUT_VAR} FALSE PARENT_SCOPE)
	endif()
endfunction()

# helper function for properties: formats a string representing the property value that will say "<none>" if the property is empty
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

# checks if the given target is some kind of CGV Framework component, and if yes, optionally returns the type of the component
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
function(cgv_gather_dependencies DEPS_LIST_OUT TARGET_NAME PROCESSED_DEPS)
	# break early if TARGET_NAME is not a CGV Framework target und thus won't provide any information we're interested in
	cgv_is_cgvtarget(IS_CGV_TARGET ${TARGET_NAME})
	if (NOT IS_CGV_TARGET)
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
	foreach (DEPENDENCY ${DEPS})
		cgv_is_cgvtarget(IS_CGV_TARGET ${DEPENDENCY})
		if (IS_CGV_TARGET AND NOT ${DEPENDENCY} IN_LIST PROCESSED_DEPS_LOCAL)
			list(APPEND DEPS_LIST_LOCAL ${DEPENDENCY})
			cgv_gather_dependencies(DEPS_LIST_LOCAL ${DEPENDENCY} PROCESSED_DEPS_LOCAL)
		endif()
	endforeach()
	list(REMOVE_DUPLICATES DEPS_LIST_LOCAL)

	# propagate results upwards
	set(${DEPS_LIST_OUT} "${DEPS_LIST_LOCAL}" PARENT_SCOPE)
	set(${PROCESSED_DEPS} "${PROCESSED_DEPS_LOCAL}" PARENT_SCOPE)
endfunction()

# filters all plugins from a list of targets
function(cgv_filter_for_plugins PLUGIN_LIST_OUT GUI_PROVIDER_PLUGIN_OUT TARGET_NAMES)
	# Make sure the variable requested to contain the GUI provider will evaluate to the FALSE value when no GUI provider is among
	# the given list of dependencies
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

# internal helper function that takes over deferred computations that require other targets to have already been fully defined
function(cgv_do_deferred_ops TARGET_NAME)
	# output notification of deferred operation
	get_target_property(TARGET_TYPE ${TARGET_NAME} CGVPROP_TYPE)
	message(STATUS "Performing deferred operations for ${TARGET_TYPE} '${TARGET_NAME}'")

	# do plugin-specific deferred operations
	if (TARGET_TYPE MATCHES "plugin$")
		# (1) gather all dependencies and compile shaderpath
		cgv_gather_dependencies(DEPENDENCIES ${TARGET_NAME} RECURSION_CONVERGENCE_HELPER)
		set(SHADER_PATHS "")
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
			set(CMD_LINE_ARGS "plugin:${GUI_PROVIDER_PLUGIN}")
			if (SHADER_PATHS)
				set(CMD_LINE_ARGS "${CMD_LINE_ARGS} \"type(shader_config):shader_path=\'${SHADER_PATHS}'\"")
			endif()
		endif()
		# --- append remaining plugins
		foreach(PLUGIN ${REQUESTED_PLUGINS})
			set(CMD_LINE_ARGS "${CMD_LINE_ARGS} plugin:${PLUGIN}")
		endforeach()
		set(CMD_LINE_ARGS "${CMD_LINE_ARGS} plugin:${TARGET_NAME}")
		# --- append custom args (if any)
		cgv_query_property(ADDITIONAL_ARGS ${TARGET_NAME} CGVPROP_ADDITIONAL_CMDLINE_ARGS)
		if (ADDITIONAL_ARGS)
			foreach(ARG ${ADDITIONAL_ARGS})
				set(ADDITIONAL_ARGS_STRING "${ADDITIONAL_ARGS_STRING} ${ARG}")
			endforeach()
			set(CMD_LINE_ARGS "${CMD_LINE_ARGS} ${ADDITIONAL_ARGS_STRING}")
		endif()

		# create a launch script in case of Make- and Ninja-based generators when the plugin is executable
		if (NOT NO_EXECUTABLE AND (CMAKE_GENERATOR MATCHES "Make" OR CMAKE_GENERATOR MATCHES "^Ninja"))
			set(WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR})
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
		else()
			# try to set relevant options for all known generators in the hopes of ending up with a valid launch/debug configuration
			if (NOT NO_EXECUTABLE)
				cgv_get_static_or_exe_name(NAME_STATIC NAME_EXE ${TARGET_NAME} TRUE)
				set_plugin_execution_params(${TARGET_NAME} ARGUMENTS ${CMD_LINE_ARGS})
				set_plugin_execution_params(${NAME_EXE} ARGUMENTS ${ADDITIONAL_ARGS_STRING} ALTERNATIVE_COMMAND $<TARGET_FILE:${NAME_EXE}>)
				set_plugin_execution_working_dir(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
				set_plugin_execution_working_dir(${NAME_EXE} ${CMAKE_CURRENT_SOURCE_DIR})
			endif()
		endif()
	endif()
endfunction()

# add a target to the build system that is decorated with several CGV Framework-specific custom properties, allowing for
# advanced features like automatic launch/debug command line generation, auto-configured single executable builds etc.
function(cgv_add_target NAME)
	cmake_parse_arguments(
		PARSE_ARGV 1 CGVARG_
		"NO_EXECUTABLE" "TYPE;OVERRIDE_SHARED_EXPORT_DEFINE;OVERRIDE_FORCE_STATIC_DEFINE"
		"SOURCES;PPP_SOURCES;HEADERS;RESOURCES;AUDIO_RESOURCES;SHADER_SOURCES;DEPENDENCIES;LINKTIME_PLUGIN_DEPENDENCIES;ADDITIONAL_CMDLINE_ARGS"
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
		shader_test(${NAME}  ST_FILES ST_INCLUDE ST_INSTALL_DIR ${CGVARG__SHADER_SOURCES})

		install(DIRECTORY ${ST_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
	endif ()
	# - generic resources (e.g. images, icons)
	if (CGVARG__RESOURCES)
		cgv_prepare_resources(${CMAKE_SOURCE_DIR} RESOURCE_SRCFILES ${CGVARG__RESOURCES})
	endif()
	# - audio resources
	if (CGVARG__AUDIO_RESOURCES AND CGV_BUILD_WITH_AUDIO)
		cgv_prepare_resources(${CMAKE_SOURCE_DIR} AUDIO_RESOURCE_SRCFILES ${CGVARG__AUDIO_RESOURCES})
	else()
		set(CGVARG__AUDIO_RESOURCES "")
	endif()

	# prepare all files for inclusion in the target 	 	 	 cgv_viewer
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
		# Moreso than indicating the type of object that will be generated from the target, the static suffix
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
	if (CGVARG__OVERRIDE_SHARED_EXPORT_DEFINE)
		target_compile_definitions(${NAME} PRIVATE "${CGVARG__OVERRIDE_SHARED_EXPORT_DEFINE}")
	else()
		target_compile_definitions(${NAME} PRIVATE "${NAME_UPPER}_EXPORTS")
	endif()
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
	set(PRIVATE_STATIC_TARGET_DEFINES "${FORCE_STATIC_DEFINE}" "REGISTER_SHADER_FILES")
	add_library(${NAME_STATIC} OBJECT ${ALL_SOURCES})
	set_target_properties(${NAME_STATIC} PROPERTIES CGVPROP_TYPE "${CGVARG__TYPE}")
	set_target_properties(${NAME_STATIC} PROPERTIES CGVPROP_SHADERPATH "${SHADER_PATH}")
	
	target_compile_definitions(${NAME_STATIC} PRIVATE ${PRIVATE_STATIC_TARGET_DEFINES})
	target_compile_definitions(${NAME_STATIC} PUBLIC "CGV_FORCE_STATIC")
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
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
		"$<BUILD_INTERFACE:${CGV_DIR}>"
		"$<BUILD_INTERFACE:${PPP_INCLUDES}>"
		"$<BUILD_INTERFACE:${ST_INCLUDE}>"
		"$<INSTALL_INTERFACE:include>")
	if (NOT IS_CORELIB)
		target_include_directories(${NAME_STATIC} PUBLIC $<BUILD_INTERFACE:${CGV_DIR}/libs>)
	endif ()

	# add single executable version if not disabled for this target
	if (IS_PLUGIN AND NOT CGVARG__NO_EXECUTABLE)
		add_executable(${NAME_EXE})
		set_target_properties(${NAME_EXE} PROPERTIES OUTPUT_NAME "${NAME}")
		target_compile_definitions(${NAME_EXE} PRIVATE ${PRIVATE_STATIC_TARGET_DEFINES} "CGV_FORCE_STATIC")
		target_include_directories(
			${NAME_EXE} PUBLIC
			"$<BUILD_INTERFACE:${CGV_DIR}>" "$<BUILD_INTERFACE:${CGV_DIR}/libs>" "$<BUILD_INTERFACE:${PPP_INCLUDES}>"
			"$<BUILD_INTERFACE:${ST_INCLUDE}>" "$<INSTALL_INTERFACE:include>"
		)
		target_link_libraries(${NAME_EXE} PRIVATE ${NAME_STATIC})
	endif()

	if (IS_PLUGIN)
		set_target_properties(${NAME} PROPERTIES CGVPROP_ADDITIONAL_CMDLINE_ARGS "${CGVARG__ADDITIONAL_CMDLINE_ARGS}")
	endif()

	# schedule deferred ops for plugins
	if (IS_PLUGIN)
		cmake_language(EVAL CODE "cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL cgv_do_deferred_ops [[${NAME}]])")
	endif()

	install(TARGETS ${NAME_STATIC} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
	if (IS_PLUGIN AND NOT CGVARG__NO_EXECUTABLE)
		install(TARGETS ${NAME_EXE} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
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

	install(TARGETS ${NAME} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})

	# Static Library
	add_library(${NAME_STATIC} OBJECT ${ALL_SOURCES})
	target_compile_definitions(${NAME_STATIC} PUBLIC "CGV_FORCE_STATIC" "REGISTER_SHADER_FILES")
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

	install(TARGETS ${NAME_STATIC} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
endfunction()
