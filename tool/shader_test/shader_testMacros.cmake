# Set CMake policy 11 to have included scripts push and pop policies
cmake_policy(SET CMP0011 NEW)
# Set CMake policy 53 for old expansion rules
cmake_policy(SET CMP0053 OLD)

file(RELATIVE_PATH SRC_NAME "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
set(LOG_BASE "${CMAKE_BINARY_DIR}/@BUILD_BASE@/log/shader/${SRC_NAME}")

macro(cgv_test_shaders target)
	if (NOT TEST_SHADERS_IGNORE)
		# Add a custom build rule for every file
		foreach (infile ${ARGN})
			shader_command_add(${target} "${infile}")
		endforeach()
	endif()
endmacro()



macro(shader_command_add target infile)

	get_filename_component(LOG_PATH "${LOG_BASE}/${infile}" PATH)
	get_filename_component(LOG_NAME "${infile}" NAME)

	get_filename_component(INFILE_ABS "${CMAKE_CURRENT_SOURCE_DIR}/${infile}" ABSOLUTE)

	set(outfile "${LOG_PATH}/${LOG_NAME}.log")

	if (NOT EXISTS "${LOG_PATH}")
		file(MAKE_DIRECTORY "${LOG_PATH}")
	endif()
	get_target_property(SHADER_TEST_COMMAND ${shader_test_EXECUTABLE} LOCATION_${CONFIG})
	add_custom_target(shader_test_${LOG_NAME}
		DEPENDS ${shader_test_EXECUTABLE} "${INFILE_ABS}"
		SOURCES "${INFILE_ABS}"
		COMMAND ${CMAKE_COMMAND} -E env "CGV_DIR=${CGV_DIR}" "CGV_OPTIONS=SHADER_DEVELOPER" ${SHADER_TEST_COMMAND} "${INFILE_ABS}" "${outfile}"
		COMMENT "Testing shader code in ${LOG_NAME}")

	add_dependencies(${target} shader_test_${LOG_NAME})
endmacro()
