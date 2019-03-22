get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

#message(FATAL_ERROR "PATH ${BASE_PATH}")

# define macros to build core components
include("${BASE_PATH}/CMakeMacroParseArguments.cmake")


macro(_cgv_install_public_headers base)
	_cgv_install_public_files(${base} ${INSTALL_HEADER_PATH} ${ARGN})
endmacro()

macro(_cgv_install_shader_files base)
	_cgv_install_public_files(${base} ${INSTALL_SHADER_PATH} ${ARGN})
endmacro()

macro(_cgv_install_resource_files base)
	_cgv_install_public_files(${base} ${INSTALL_RESOURCE_PATH} ${ARGN})
endmacro()


macro(_cgv_install_public_files base destbase)
	set(HEADERS ${ARGN})
	if (HEADERS)
		foreach(header ${HEADERS})
			if (IS_ABSOLUTE ${header})
				# The header path is absolute. Check if this header is inside the build folder
				set(HB_PATH "${CMAKE_BINARY_DIR}/${BUILD_BASE}/${destbase}/${base}")
				file(RELATIVE_PATH PUBH_REL "${HB_PATH}" "${header}")

				if (IS_ABSOLUTE "${PUBH_REL}")
					message(FATAL_ERROR "Cannot install file ${header} as it has an absolute path that is outside of the project binaries dir!")
				endif()
				get_filename_component(PUBH_REL "${PUBH_REL}" PATH)

			else()
				get_filename_component(PUBH_REL "${header}" PATH)			
			endif()
			install(FILES ${header} DESTINATION "${INSTALL_BASE}/${destbase}/${base}/${PUBH_REL}")
		endforeach()		
	endif()
endmacro()



macro(_cgv_set_cxx_standard target)
        # Set the language standard
        # FIXME: The following will not work in windows. The correct approach would be
        # set_property(TARGET ${target_name} PROPERTY CXX_STANDARD ${CXX_STANDARD})	
        # which is not supported. So for now we hack this property in
        set(CMAKE_CXX_FLAGS "-std=c++11")
endmacro()


macro(_cgv_set_definitions target)
	parse_arguments(DEFS "SHARED;STATIC;COMMON" "" ${ARGN})

	set(${target}_SHARED_DEFINITIONS ${${target}_SHARED_DEFINITIONS} ${DEFS_COMMON} ${DEFS_SHARED})
	set(${target}_STATIC_DEFINITIONS ${${target}_STATIC_DEFINITIONS} ${DEFS_COMMON} ${DEFS_STATIC})
	set(${target}_COMMON_DEFINITIONS ${${target}_COMMON_DEFINITIONS} ${DEFS_COMMON} ${DEFS_STATIC})
	
	if (${target}_SHARED_DEFINITIONS)
		list(REMOVE_DUPLICATES ${target}_SHARED_DEFINITIONS)
	endif()
	if (${target}_STATIC_DEFINITIONS)
		list(REMOVE_DUPLICATES ${target}_STATIC_DEFINITIONS)
	endif()
	
	set_target_properties(${target} PROPERTIES
		_SHARED_DEFINITIONS "${${target}_SHARED_DEFINITIONS}"
		_STATIC_DEFINITIONS "${${target}_STATIC_DEFINITIONS}"
		_COMMON_DEFINITIONS "${${target}_COMMON_DEFINITIONS}")
		
	if (BUILD_SHARED_LIBS)
		set_target_properties(${target} PROPERTIES COMPILE_DEFINITIONS "${${target}_SHARED_DEFINITIONS}")
	else()
		set_target_properties(${target} PROPERTIES COMPILE_DEFINITIONS "${${target}_STATIC_DEFINITIONS}")
	endif()
endmacro()

macro(cgv_add_export_definitions target)

	set(SHARED_DEFS "")
	set(STATIC_DEFS "")
	
	foreach(def ${ARGN})
		string(TOUPPER "${def}" DEF_UPPER)
		set(SHARED_DEFS ${SHARED_DEFS} "${DEF_UPPER}_EXPORTS")
		set(STATIC_DEFS ${STATIC_DEFS} "${DEF_UPPER}_FORCE_STATIC")
	endforeach()

	_cgv_set_definitions(${target}
		SHARED ${SHARED_DEFS}
		STATIC ${STATIC_DEFS})
endmacro()


macro(cgv_add_module target)

	parse_arguments(DEFS "SOURCES;SHADERS;FILES" "" ${ARGN})

	if (NOT DEFS_SOURCES AND NOT DEFS_SHADERS AND NOT DEFS_FILES)
		set(DEFS_SOURCES ${ARGN})
		set(DEFS_SHADERS "")
		set(DEFS_FILES "")
	endif()

	if (BUILD_SHARED_LIBS)
		add_library(${target} MODULE ${DEFS_SOURCES})
	else()
		add_library(${target} STATIC ${DEFS_SOURCES})
	endif()

	string(TOUPPER ${target} TARGET_UPPER)
	_cgv_set_definitions(${target}
		COMMON _UNICODE UNICODE
		SHARED "${TARGET_UPPER}_EXPORTS";"_USRDLL"
		STATIC "${TARGET_UPPER}_FORCE_STATIC;CGV_FORCE_STATIC")
		
	set_target_properties(${target} PROPERTIES FOLDER "${FOLDER_NAME_PLUGINS}")
	install(TARGETS ${target} DESTINATION "${INSTALL_BASE}/${INSTALL_LIB_PATH}")
	_cgv_install_shader_files("${target}" ${DEFS_SHADERS})
	_cgv_install_resource_files("${target}" ${DEFS_FILES})

    # Set the cxx standard
	_cgv_set_cxx_standard(${target})

	# Remember the type for later config generation
	set_target_properties(${target} PROPERTIES HEADER_LOCAL_PATH "plugins")	
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

    # Set the language standard
    _cgv_set_cxx_standard(${target_name})	


	# Remember the type for later config generation
	set_target_properties(${target_name} PROPERTIES HEADER_LOCAL_PATH "apps")
endmacro()


macro(_cgv_add_custom_library target_name folder)
	# Parse the argument list
	parse_arguments(CONF "SOURCES;HEADERS;PUBLIC_HEADERS;SHARED_DEFINITIONS;STATIC_DEFINITIONS;DEFINITIONS;SHADERS;FILES;INSTALL_NAME" "" ${ARGN})

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
	_cgv_install_shader_files("${CONF_INSTALL_NAME}" ${CONF_SHADERS})
	_cgv_install_resource_files("${CONF_INSTALL_NAME}" ${CONF_FILES})
	

	# Remember the type and install name for later config generation
	set_target_properties(${target_name} PROPERTIES 
		HEADER_LOCAL_PATH "libs"
		HEADER_LOCAL_NAME ${CONF_INSTALL_NAME})

    # Set the language standard
    _cgv_set_cxx_standard(${target_name})	


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


# Include macros that deal with the creation of config files
include("${BASE_PATH}/createConfigScript.cmake")
