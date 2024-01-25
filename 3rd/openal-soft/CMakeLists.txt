
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

# Disable warnings we can't do anything about since it would require changing the original source code
if (CMAKE_C_COMPILER_ID MATCHES "GNU" OR MAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(OpenAL PRIVATE -Wno-stringop-overflow -Wno-inline)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang" OR MAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(OpenAL PRIVATE -Wno-unused-but-set-variable)
endif()

# tack on IDE fluff
cgv_get_all_directory_targets(OAL_TARGETS ${CMAKE_CURRENT_LIST_DIR} RECURSIVE)
set_target_properties(${OAL_TARGETS} PROPERTIES FOLDER "3rd/openal")
