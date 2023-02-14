
# preset options
option(ALSOFT_EAX ON)
option(ALSOFT_EXAMPLES OFF)
option(ALSOFT_INSTALL_EXAMPLES OFF)
option(ALSOFT_NO_CONFIG_UTIL ON)
option(ALSOFT_INSTALL_CONFIG OFF)

# include the OpenAL-soft build tree
add_subdirectory(openal-soft)

# tack on IDE fluff
cgv_get_all_directory_targets(OAL_TARGETS ${CMAKE_CURRENT_LIST_DIR} RECURSIVE)
set_target_properties(${OAL_TARGETS} PROPERTIES FOLDER "3rd/openal")
