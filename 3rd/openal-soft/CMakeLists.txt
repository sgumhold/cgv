
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

# tack on IDE fluff
cgv_get_all_directory_targets(OAL_TARGETS ${CMAKE_CURRENT_LIST_DIR} RECURSIVE)
set_target_properties(${OAL_TARGETS} PROPERTIES FOLDER "3rd/openal")
