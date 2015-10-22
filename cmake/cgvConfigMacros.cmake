get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)

# define macros to build core components
include("${BASE_PATH}/CMakeMacroParseArguments.cmake")

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

	if (BUILD_SHARED_LIBS)
		add_library(${target} MODULE ${ARGN})
	else()
		add_library(${target} STATIC ${ARGN})
	endif()

	string(TOUPPER ${target} TARGET_UPPER)
	_cgv_set_definitions(${target}
		COMMON _UNICODE UNICODE
		SHARED "_USRDLL"
		STATIC "${TARGET_UPPER}_FORCE_STATIC;CGV_FORCE_STATIC")
		
	set_target_properties(${target} PROPERTIES FOLDER "${FOLDER_NAME_PLUGINS}")
	install(TARGETS ${target} DESTINATION "${INSTALL_BASE}/${INSTALL_LIB_PATH}")

    # Set the cxx standard
	_cgv_set_cxx_standard(${target})

	# Remember the type for later config generation
	set_target_properties(${target} PROPERTIES HEADER_LOCAL_PATH "plugins")	
endmacro()




# Include macros that deal with the creation of modules, config files etc
include("${BASE_PATH}/createConfigScript.cmake")
