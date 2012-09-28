get_filename_component(SRC_NAME "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
set(LOG_BASE "${CMAKE_BINARY_DIR}/@BUILD_BASE@/log/shader/${SRC_NAME}")


macro(test_shaders)
	if (NOT TEST_SHADERS_IGNORE)
		# Add a custom build rule for every file
		foreach (infile ${ARGN})
			shader_command_add("${infile}")
		endforeach()
	endif()
endmacro()



macro(shader_command_add infile)

	get_filename_component(LOG_PATH "${LOG_BASE}/${infile}" PATH)
	get_filename_component(LOG_NAME "${infile}" NAME)

	set(outfile "${LOG_PATH}/${LOG_NAME}.log")

	if (NOT EXISTS "${LOG_PATH}")
		file(MAKE_DIRECTORY "${LOG_PATH}")
	endif()
	add_custom_command(OUTPUT "${outfile}"
		COMMAND shader_test
		ARGS "${CMAKE_CURRENT_SOURCE_DIR}/${infile}" "${outfile}"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${infile}")
endmacro()
