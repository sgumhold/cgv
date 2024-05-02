
# preset options
option(BUILD_PROGRAMS "" OFF)
option(BUILD_EXAMPLES "" OFF)
option(BUILD_TESTING "" OFF)

# include the libsndfile build tree
add_subdirectory(libsndfile)

# Disable warnings we can't do anything about since it would require changing the original source code
if (CMAKE_C_COMPILER_ID MATCHES "GNU" OR MAKE_CXX_COMPILER_ID MATCHES "GNU")
	target_compile_options(sndfile PRIVATE -Wno-format-truncation)
endif()

# tack on IDE fluff
cgv_get_all_directory_targets(SNDFILE_TARGETS ${CMAKE_CURRENT_LIST_DIR} RECURSIVE)
set_property(TARGET ${SNDFILE_TARGETS} PROPERTY FOLDER "3rd/sndfile")
