
# preset options
if (NOT DEFINED CACHE{ALSOFT_EAX})
	# this option weirdly force-defaults to OFF, we want it to default to ON though
	set(ALSOFT_EAX ON CACHE BOOL "" FORCE)
endif()
option(ALSOFT_EXAMPLES OFF)
option(ALSOFT_INSTALL_EXAMPLES OFF)
if (NOT DEFINED CACHE{ALSOFT_NO_CONFIG_UTIL})
	# this option weirdly force-defaults to OFF, we want it to default to ON though
	set(ALSOFT_NO_CONFIG_UTIL ON CACHE BOOL "" FORCE)
endif()
option(ALSOFT_INSTALL_CONFIG OFF)
option(ALSOFT_UTILS OFF)
option(ALSOFT_INSTALL_UTILS OFF)

# include the OpenAL-soft build tree
add_subdirectory(openal-soft)

# log all targets added by the 3rd party lib
cgv_get_all_directory_targets(OAL_TARGETS ${CMAKE_CURRENT_LIST_DIR} RECURSIVE)

# Disable warnings we can't do anything about since it would require changing the original source code
foreach(OAL_TGT ${OAL_TARGETS})
	# check wether we can add flags to the target
	get_target_property(TGT_TYPE ${OAL_TGT} TYPE)
	if (NOT TGT_TYPE STREQUAL "UTILITY")
		if (CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
			target_compile_options(${OAL_TGT} PRIVATE -Wno-stringop-overflow -Wno-inline)
		elseif(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
			target_compile_options(${OAL_TGT} PRIVATE -Wno-everything)
		endif()
	endif()
endforeach()

# tack on IDE fluff
set_target_properties(${OAL_TARGETS} PROPERTIES FOLDER "3rd/openal")
