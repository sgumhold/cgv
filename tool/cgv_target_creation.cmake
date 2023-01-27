
# helper function for properties: formats a string representing the property value that will say "<none>" if the property is empty
function(cgv_format_property_value OUTPUT_VAR TARGET_NAME)
	cmake_parse_arguments(
		PARSE_ARGV 2 CGVARG_ "" "PROPERTY" ""
	)
	if (NOT CGVARG__PROPERTY)
		message(FATAL_ERROR "cgv_format_property_value(): no property specified!")
	endif()

	get_target_property(PROPVAL ${TARGET_NAME} ${CGVARG__PROPERTY})
	set(${OUTPUT_VAR} "<none>" PARENT_SCOPE)
	if (PROPVAL)
		set(${OUTPUT_VAR} "${PROPVAL}" PARENT_SCOPE)
	endif()
endfunction()


# internal helper function for recursively gathering transitive CGV Framework component dependencies of a target
function(cgv_gather_dependencies DEPS_LIST_OUT TARGET_NAME PROCESSED_DEPS)
	# break early if TARGET_NAME is not a target known to CMake so we don't try to process explicit linker objects
	if (NOT TARGET ${TARGET_NAME})
		message("     ...not a CMake target")
		return()
	endif()

	# break early if TARGET_NAME is not a CGV Framework target und thus won't provide any information we're interested in
	get_target_property(TARGET_TYPE ${TARGET_NAME} CGVPROP_TYPE)
	if (TARGET_TYPE STREQUAL "TARGET_TYPE-NOTFOUND")
		message("   ...not a CGV target")
		return()
	endif()

	# make sure already-processed dependencies can be propagated, and break the recursion
	# cycle for this target by adding it to the list
	set(DEPS_LIST_LOCAL "${${DEPS_LIST_OUT}}")
	set(PROCESSED_DEPS_LOCAL "${${PROCESSED_DEPS}}")
	message("# PROCESSED_DEPS before: ${PROCESSED_DEPS_LOCAL}")
	list(APPEND PROCESSED_DEPS_LOCAL ${TARGET_NAME})
	message("# PROCESSED_DEPS   now : ${PROCESSED_DEPS_LOCAL}")

	# gather direct dependencies (TODO: may contain generator expressions - we currently ignore those)
	message("- gathering dependencies for '${TARGET_NAME}'")
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
	message(" = [${DEPS}]")

	# recursive expansion
	foreach (DEPENDENCY ${DEPS})
		message("   + considering '${DEPENDENCY}'")
		if (NOT ${DEPENDENCY} IN_LIST PROCESSED_DEPS_LOCAL)
			list(APPEND DEPS_LIST_LOCAL ${DEPENDENCY})
			cgv_gather_dependencies(DEPS_LIST_LOCAL ${DEPENDENCY} PROCESSED_DEPS_LOCAL)
		endif()
	endforeach()
	list(REMOVE_DUPLICATES DEPS_LIST_LOCAL)

	# propagate processed dependencies upwards
	set(${DEPS_LIST_OUT} "${DEPS_LIST_LOCAL}" PARENT_SCOPE)
	set(${PROCESSED_DEPS} "${PROCESSED_DEPS_LOCAL}" PARENT_SCOPE)
endfunction()

# internal helper function for recursively gathering transitively requested plugins of a target
function(cgv_gather_requested_plugins PLUGIN_LIST_OUT TARGET_NAME PROCESSED_PLUGINS)
	# break recursion cycle for this plugin
	set(PROCESSED_PLUGINS_LIST ${PROCESSED_PLUGINS})
	list(APPEND PROCESSED_PLUGINS_LIST ${TARGET_NAME})

	# gather plugins requested by this plugin
	#message(" - gathering requested plugins for '${TARGET_NAME}'")
	get_target_property(PLUGINS ${TARGET_NAME} CGVPROP_REQUESTED_PLUGINS)
	if (PLUGINS STREQUAL "PLUGINS-NOTFOUND")
		#message("   <none>")
	else()
		#message("   [${PLUGINS}]")
		foreach (PLUGIN ${PLUGINS})
			#message("   + considering '${PLUGIN}'")
			if (NOT ${PLUGIN} IN_LIST PROCESSED_PLUGINS_LIST)
				list(APPEND PLUGIN_LIST_OUT ${PLUGIN})
				cgv_gather_requested_plugins(PLUGIN_LIST_OUT ${PLUGIN} ${PROCESSED_PLUGINS_LIST})
			endif()
		endforeach()
	endif()
endfunction()

# internal helper function that takes over deferred computations that require other targets to have already been declared
function(cgv_do_deferred_ops TARGET_NAME)
	# output notification of deferred operation
	get_target_property(TARGET_TYPE ${TARGET_NAME} CGVPROP_TYPE)
	message("Performing deferred operations for ${TARGET_TYPE} '${TARGET_NAME}'")

	# compile command line for launching plugins
	if (TARGET_TYPE MATCHES "plugin$")

		# (1) gather all dependencies
		cgv_gather_dependencies(DEPENDENCIES ${TARGET_NAME} RECURSION_CONVERGENCE_HELPER)
		# DEBUG: check for duplicates
		message(" --------------------------")
		message("DEPENDENCIES (a): ${DEPENDENCIES}")
		list(REMOVE_DUPLICATES DEPENDENCIES)
		message("DEPENDENCIES (b): ${DEPENDENCIES}")
		message("  -----")
		message("RECURSION LIST (a): ${RECURSION_CONVERGENCE_HELPER}")
		list(REMOVE_DUPLICATES RECURSION_CONVERGENCE_HELPER)
		message("RECURSION LIST (b): ${RECURSION_CONVERGENCE_HELPER}")
		message("################################")
		message("################################")
		cgv_gather_requested_plugins(REQUESTED_PLUGINS ${TARGET_NAME} RECURSION_CONVERGENCE_HELPER)

		# (2) compile command line arguments
		set(CMD_ARGS "")
		foreach(PLUGIN ${REQUESTED_PLUGINS})
			list(APPEND CMD_ARGS "plugin:${PLUGIN}")
		endforeach()
	endif()

	# create a launch script in case of Make- and Ninja-based generators
	if (CMAKE_GENERATOR MATCHES "Make" OR CMAKE_GENERATOR MATCHES "^Ninja")
		configure_file(
			"${CGV_DIR}/make/cmake/run_plugin.sh.in" "${CMAKE_BINARY_DIR}/run_${NAME}.sh"
			FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
			@ONLY
		)
	endif()
endfunction()

# add a target to the build system that is decorated with several CGV Framework-specific custom properties, allowing for
# advanced features like automatic launch/debug command line generation, resource embedding for single executable builds etc.
function(cgv_add_target NAME)
	cmake_parse_arguments(
		PARSE_ARGV 1 CGVARG_
		"" "TYPE"
		"SOURCES;PPP_SOURCES;HEADERS;RESOURCES;AUDIO_RESOURCES;SHADER_SOURCES;DEPENDENCIES"
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
		foreach (SHADER_SOURCE ${CGVARG__SHADER_SOURCES})
			cmake_path(
				ABSOLUTE_PATH SHADER_SOURCE BASE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} NORMALIZE
				OUTPUT_VARIABLE SHADER_SOURCE_ABSOLUTE_PATH)
			cmake_path(GET SHADER_SOURCE_ABSOLUTE_PATH PARENT_PATH SHADER_SOURCE_PARENT_PATH)
			list(APPEND SHADER_PATH ${SHADER_SOURCE_PARENT_PATH})
		endforeach()
		list(REMOVE_DUPLICATES SHADER_PATH)

		shader_test("${CGV_DIR}"
				ST_FILES
				ST_INCLUDES
				ST_INSTALL_DIR
				${CGVARG__SHADER_SOURCES})

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

	# determine target names for shared and static builds
	string(TOUPPER ${NAME} NAME_UPPER)
	set(NAME_STATIC ${NAME}_static)
	set(ALL_SOURCES
		${CGVARG__SOURCES} ${PPP_FILES} ${CGVARG__PPP_SOURCES} ${CGVARG__HEADERS} ${ST_FILES} ${SHADERS}
		${RESOURCE_SRCFILES} ${CGVARG__RESOURCES}
	)
	if (CGV_BUILD_WITH_AUDIO)
		list(APPEND ALL_SOURCES ${AUDIO_RESOURCE_SRCFILES} ${CGVARG__AUDIO_RESOURCES})
	endif()

	# for shared builds
	if (IS_STATIC)
		# pure static libraries do not produce DLLs/shared objects, but they still need a non-static target
		# for providing apps/DLLs/shared objects with information how they can (statically) link to them
		add_library(${NAME} ${ALL_SOURCES})
	else()
		add_library(${NAME} SHARED ${ALL_SOURCES})
	endif()

	set_target_properties(${NAME} PROPERTIES CGVPROP_TYPE "${CGVARG__TYPE}")
	set_target_properties(${NAME} PROPERTIES CGVPROP_SHADERPATH "${SHADER_PATH}")
	target_compile_definitions(${NAME} PRIVATE ${NAME_UPPER}_EXPORTS)
	foreach (DEPENDENCY ${CGVARG__DEPENDENCIES})
		# find out dependency type
		get_target_property(DEPENDENCY_TYPE ${DEPENDENCY} CGVPROP_TYPE)
		# handle linker dependencies:
		# link in all libraries, and also blanket-include them as transitive link dependencies (for now... this
		# is not ideal and might be changed later)
		if (DEPENDENCY_TYPE STREQUAL "library" OR DEPENDENCY_TYPE STREQUAL "corelib")
			target_link_libraries(${NAME} PUBLIC ${DEPENDENCY})
		# handle remaining types of dependencies
		else()
			# if we are a plugin, check all dependencies whether they are also a plugin and if yes, note them
			# down as a plugin dependency (used e.g. for launch/debug command line generation)
			if (IS_PLUGIN AND DEPENDENCY_TYPE MATCHES "plugin$")
				set_property(TARGET ${NAME} APPEND PROPERTY CGVPROP_REQUESTED_PLUGINS ${DEPENDENCY})
			endif()
			add_dependencies(${NAME} ${DEPENDENCY})
		endif()
	endforeach()

	target_include_directories(${NAME} PUBLIC
			$<BUILD_INTERFACE:${CGV_DIR}>
			"$<BUILD_INTERFACE:${PPP_INCLUDES}>"
			"$<BUILD_INTERFACE:${ST_INCLUDES}>")
	if (IS_CORELIB)
		target_include_directories(${NAME} PUBLIC $<INSTALL_INTERFACE:include>)
	else ()
		target_include_directories(${NAME} PUBLIC
				$<BUILD_INTERFACE:${CGV_DIR}/libs>
				$<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
	endif ()

	install(TARGETS ${NAME} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})

	# for static builds
	add_library(${NAME_STATIC} STATIC ${ALL_SOURCES})
	set_target_properties(${NAME_STATIC} PROPERTIES CGVPROP_TYPE "${CGVARG__TYPE}")
	set_target_properties(${NAME_STATIC} PROPERTIES CGVPROP_SHADERPATH "${SHADER_PATH}")
	target_compile_definitions(${NAME_STATIC} PUBLIC CGV_FORCE_STATIC)
	foreach (DEPENDENCY ${CGVARG__DEPENDENCIES})
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
	if (NOT IS_CORELIB)
		target_include_directories(${NAME_STATIC} PUBLIC $<BUILD_INTERFACE:${CGV_DIR}/libs>)
	endif ()

	# /BEGIN DEBUG OUTPUT
	get_target_property(PROPVAL_TYPE ${NAME} CGVPROP_TYPE)
	message(STATUS "CGV TARGET: ${NAME} (${PROPVAL_TYPE})")
	cgv_format_property_value(PROPVAL_SP ${NAME} PROPERTY CGVPROP_SHADERPATH)
	message(STATUS " [ shaderpath ]  ${PROPVAL_SP}")
	if (IS_PLUGIN)
		cgv_format_property_value(PROPVAL_RP ${NAME} PROPERTY CGVPROP_REQUESTED_PLUGINS)
		message(STATUS " [req. plugins]  ${PROPVAL_RP}")
	endif()
	cgv_format_property_value(PROPVAL_LDEPS ${NAME} PROPERTY LINK_LIBRARIES)
	message(STATUS " [ linked libs]  ${PROPVAL_LDEPS}")
	cgv_format_property_value(PROPVAL_CDEFS ${NAME} PROPERTY COMPILE_DEFINITIONS)
	message(STATUS " [compile defs]  ${PROPVAL_CDEFS}")
	# /END DEBUG OUTPUT

	# schedule generating deferred ops for plugins
	if (IS_PLUGIN)
		cmake_language(EVAL CODE "cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL cgv_do_deferred_ops [[${NAME}]])")
	endif()

	install(TARGETS ${NAME_STATIC} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
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
		shader_test("${CGV_DIR}"
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
	target_compile_definitions(${NAME} PRIVATE ${NAME_UPPER}_EXPORTS)
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
	add_library(${NAME_STATIC} STATIC ${ALL_SOURCES})
	target_compile_definitions(${NAME_STATIC} PUBLIC CGV_FORCE_STATIC)
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
