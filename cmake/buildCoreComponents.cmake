# Only include file once
if (BUILD_CORE_COMPONENTS)
	return()
endif()
set(BUILD_CORE_COMPONENTS TRUE)

# Set build settings for the standard elements of the framework bundle
get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${BASE_PATH}/buildSettings.cmake")
include("${BASE_PATH}/cgvConfigMacros.cmake")

# define macros to build core components
include("${BASE_PATH}/CMakeMacroParseArguments.cmake")

macro(_cgv_define_source_group basename)
	foreach(source ${ARGN})
		if (IS_ABSOLUTE ${source})
			source_group(${basename} FILES ${source})
		else()
			get_filename_component(fpath "${source}" PATH)
			if (fpath)
				set(fpath "${fpath}/")
			endif()
			string(REPLACE "/" "\\" fpath "${fpath}")
			source_group("${fpath}${basename}" FILES ${source})
		endif()
	endforeach()
endmacro()


macro(_cgv_add_core_library target_name)
	# Parse the argument list
	parse_arguments(COMPONENT "SOURCES;HEADERS;PUBLIC_HEADERS;CGV_DEPENDENCIES;SHARED_DEFINITIONS;STATIC_DEFINITIONS" "HEADERS_ONLY" ${ARGN})

	# Do nothing if this component only consists of headers
	if (NOT COMPONENT_HEADERS_ONLY)
		# Expand the dependencies by prepending a "cgv_" to every element
		set(CGV_DEPENDENCIES_EXP "")
		foreach(dep ${COMPONENT_CGV_DEPENDENCIES})
			list(APPEND CGV_DEPENDENCIES_EXP "cgv_${dep}")
		endforeach()

		# Set include directory
		include_directories(${CMAKE_CURRENT_SOURCE_DIR}
							"${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_HEADER_PATH}")

		# Add a library
		add_library(${target_name} ${COMPONENT_SOURCES} ${COMPONENT_HEADERS} ${COMPONENT_PUBLIC_HEADERS})

		# Set dependent components
		target_link_libraries(${target_name} ${CGV_DEPENDENCIES_EXP})

		# Create some filters for IDEs
		_cgv_define_source_group("Sources" ${COMPONENT_SOURCES})
		_cgv_define_source_group("Headers" ${COMPONENT_HEADERS} ${COMPONENT_PUBLIC_HEADERS})
	#	source_group("Sources" FILES ${COMPONENT_SOURCES})
	#	source_group("Headers" FILES ${COMPONENT_HEADERS} ${COMPONENT_PUBLIC_HEADERS})

		# Create installation rules for the target
		install(TARGETS ${target_name} DESTINATION "${INSTALL_BASE}/${INSTALL_LIB_PATH}" EXPORT ${CGV_EXPORTS_ID})

		# Append this element to the list of targets
	#	set(CGV_LIB_TARGETS ${CGV_LIB_TARGETS} ${target_name} CACHE INTERNAL "" FORCE)
		set(CGV_LIB_TARGETS ${CGV_LIB_TARGETS} ${target_name} PARENT_SCOPE)
	
		# Set a source folder for VisualStudio
		set_target_properties(${target_name} PROPERTIES FOLDER "${FOLDER_NAME_CGV}")
	
		# Set definitions for shared and static libraries.
		string(TOUPPER ${target_name} TARGET_UPPER)
		
		_cgv_set_definitions(${target_name}
			COMMON UNICODE _UNICODE
			SHARED "${TARGET_UPPER}_EXPORTS" ${COMPONENT_SHARED_DEFINITIONS}
			STATIC "${TARGET_UPPER}_FORCE_STATIC;CGV_FORCE_STATIC" ${COMPONENT_STATIC_DEFINITIONS})
	
		# Add definitions to use unicode
		# FIXME: This might be a hack
		add_definitions("-DUNICODE -D_UNICODE")
	endif()
endmacro()


macro(_cgv_install_public_headers base)
	set(HEADERS ${ARGN})
	if (HEADERS)
		foreach(header ${HEADERS})
			if (IS_ABSOLUTE ${header})
				# The header path is absolute. Check if this header is inside the build folder
				set(HB_PATH "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_HEADER_PATH}/${base}")
				file(RELATIVE_PATH PUBH_REL "${HB_PATH}" "${header}")

				if (IS_ABSOLUTE "${PUBH_REL}")
					message(FATAL_ERROR "Cannot install header ${header} as it has an absolute path that is outside of the project binaries dir!")
				endif()
				get_filename_component(PUBH_REL "${PUBH_REL}" PATH)

			else()
				get_filename_component(PUBH_REL "${header}" PATH)			
			endif()
			install(FILES ${header} DESTINATION "${INSTALL_BASE}/${INSTALL_HEADER_PATH}/${base}/${PUBH_REL}")
		endforeach()		
	endif()
endmacro()


macro(cgv_add_core_component target_name)

	# a component is an expanded cgv-library
	_cgv_add_core_library(cgv_${target_name} ${ARGN})
#	set_target_properties(cgv_${target_name} PROPERTIES LINK_FLAGS -nostdlib)

	# Create installation rules for the public headers
	_cgv_install_public_headers("cgv/${target_name}" ${COMPONENT_PUBLIC_HEADERS})
endmacro()


macro(cgv_add_executable target_name)
	add_executable(${target_name} ${ARGN})
	_cgv_set_definitions(${target_name}
		COMMON UNICODE _UNICODE
		STATIC CGV_FORCE_STATIC)
	install(TARGETS ${target_name} DESTINATION "${INSTALL_BASE}/${INSTALL_BIN_PATH}" EXPORT ${target_name}Depends)
	
	# There is a bug in CMake in which postfixes are not automatically added.
	set_target_properties(${target_name} PROPERTIES
		OUTPUT_NAME ${target_name}
		RELEASE_OUTPUT_NAME ${target_name}
		DEBUG_OUTPUT_NAME ${target_name}${DEBUG_POSTFIX})	

	# Set the relative path of needed dynamic libraries directly inside
	# the binary on supported plattforms
	file(RELATIVE_PATH REL_BIN_TO_LIB "/${INSTALL_BASE}/${INSTALL_BIN_PATH}" "/${INSTALL_BASE}/${INSTALL_LIB_PATH}")
	set_target_properties(${target_name} PROPERTIES INSTALL_RPATH "\$ORIGIN/${REL_BIN_TO_LIB}")

	# Remember the type for later config generation
	set_target_properties(${target_name} PROPERTIES HEADER_LOCAL_PATH "apps")
endmacro()


macro(_cgv_add_custom_library target_name folder)
	# Parse the argument list
	parse_arguments(CONF "SOURCES;HEADERS;PUBLIC_HEADERS;SHARED_DEFINITIONS;STATIC_DEFINITIONS;DEFINITIONS;INSTALL_NAME" "" ${ARGN})

	if (NOT CONF_INSTALL_NAME)
		set(CONF_INSTALL_NAME ${target_name})
	endif()	

	# Add a library
	add_library(${target_name} ${CONF_SOURCES} ${CONF_HEADERS} ${CONF_PUBLIC_HEADERS})

	# Set include directories
	include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	
	# Create installation rules for the target
	install(TARGETS ${target_name} DESTINATION "${INSTALL_BASE}/${INSTALL_LIB_PATH}")
	_cgv_install_public_headers("libs/${CONF_INSTALL_NAME}" ${CONF_PUBLIC_HEADERS})

	# Remember the type and install name for later config generation
	set_target_properties(${target_name} PROPERTIES 
		HEADER_LOCAL_PATH "libs"
		HEADER_LOCAL_NAME ${CONF_INSTALL_NAME}
)

	# Set a source folder for VisualStudio
	set_target_properties(${target_name} PROPERTIES FOLDER "${folder}")
	
	_cgv_set_definitions(${target_name}
		COMMON "${CONF_DEFINITIONS}"
		SHARED "${CONF_SHARED_DEFINITIONS}"
		STATIC "${CONF_STATIC_DEFINITIONS};CGV_FORCE_STATIC")
endmacro()


macro(cgv_add_library target_name)
	_cgv_add_custom_library(${target_name} "${FOLDER_NAME_LIBS}" "${ARGN}")
endmacro()

	
macro(cgv_add_3rd_library target_name)
	_cgv_add_custom_library(${target_name} "${FOLDER_NAME_3RD}" "${ARGN}")
endmacro()



macro(build_core_components_finish)

	# HACK: Since CMake has no way to enumerate targets they are written
	# in a cmake file which will also be installed
	SET(LOCAL_EXPORTS_DIR "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}")
	
	set(CGV_TARGETS_DEFINITIONS "")
	foreach(target ${CGV_LIB_TARGETS})
		get_target_property(SHARED_DEF ${target} _SHARED_DEFINITIONS)
		get_target_property(STATIC_DEF ${target} _STATIC_DEFINITIONS)
		set(CGV_TARGETS_DEFINITIONS "${CGV_TARGETS_DEFINITIONS}\nset(${target}_SHARED_DEFINITIONS \"${SHARED_DEF}\")")
		set(CGV_TARGETS_DEFINITIONS "${CGV_TARGETS_DEFINITIONS}\nset(${target}_STATIC_DEFINITIONS \"${STATIC_DEF}\")")		
	endforeach()
	
	
	
	configure_file("${CGV_BASE}/cmake/${CGV_TARGETS_CONFIG_NAME}.in"
		"${LOCAL_EXPORTS_DIR}/${CGV_TARGETS_CONFIG_NAME}"
		@ONLY)
	
#	file(WRITE "${LOCAL_EXPORTS_DIR}/${CGV_EXPORTS_INDEX_NAME}"
#	"SET(CGV_TARGETS ${CGV_LIB_TARGETS})")
#	install(FILES "${LOCAL_EXPORTS_DIR}/${CGV_EXPORTS_INDEX_NAME}"
#			DESTINATION "${INSTALL_BASE}/${INSTALL_CMAKE_PATH}")

	export(TARGETS ${CGV_LIB_TARGETS} FILE "${LOCAL_EXPORTS_DIR}/${CGV_EXPORTS_NAME}")

	# Export all targets 
	install(EXPORT ${CGV_EXPORTS_ID} DESTINATION "${INSTALL_BASE}/${INSTALL_CMAKE_PATH}")

	install(FILES	${CGV_BASE}/cmake/base.cmake
					${CGV_BASE}/cmake/buildCoreComponents.cmake
					${CGV_BASE}/cmake/buildSettings.cmake
					${CGV_BASE}/cmake/cgvConfig.cmake
					${CGV_BASE}/cmake/cgvConfigGeneral.cmake
					${CGV_BASE}/cmake/cgvConfigMacros.cmake
					${CGV_BASE}/cmake/cgvConfigVersion.cmake
					${CGV_BASE}/cmake/cgvTargetsConfig.cmake.in
					${CGV_BASE}/cmake/CMakeMacroParseArguments.cmake
					${CGV_BASE}/cmake/createConfigScript.cmake
					${CGV_BASE}/cmake/FindPkg.cmake.in
					${CGV_BASE}/cmake/PkgBinConfig.cmake.in
					${CGV_BASE}/cmake/PkgLibConfig.cmake.in
			DESTINATION "${INSTALL_BASE}/cmake")
	
	# TODO: Comment this
	set(LOCAL_BUILD_ANNOUNCED TRUE)
	set(LOCAL_BUILD_DIR "${CMAKE_BINARY_DIR}/${BUILD_BASE}")

	file(WRITE "${CGV_BASE}/cmake/local_build_dir.cmake"
		 "set(LOCAL_BUILD_DIR \"${LOCAL_BUILD_DIR}\")")
		
	set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
		"${CGV_BASE}/cmake/local_build_dir.cmake")	
endmacro()




# Macro to export the find-File
#macro(cgv_export_package_file target)
#
#	set(EXPORT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
#	set(EXPORT_FILE "${EXPORT_PATH}/${target}Config.cmake")
#
#	# Install this file for the installation target
#	install(FILES "${EXPORT_FILE}" 
#			DESTINATION "${INSTALL_BASE}/${INSTALL_CMAKE_PATH}")
#
#	# It might be possible that this find-file is a sub project of a larger
#	# project where other sub project already require the file. Make it
#	# accessible in the source directory through the ${target}_DIR variable
#	# (which is automatically checked by cmake)
#	# set(${target}_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)
#	set(${target}_DIR "${EXPORT_PATH}" CACHE INTERNAL "" FORCE)
#
#	file(APPEND "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${INSTALL_CMAKE_PATH}/local_config.cmake" 
#		 "set(${target}_DIR \"${EXPORT_PATH}\")\n")
#endmacro()


