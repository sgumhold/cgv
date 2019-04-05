# Set CMake policy 11 to have included scripts push and pop policies
cmake_policy(SET CMP0011 NEW)
# Set CMake policy 53 for old expansion rules
cmake_policy(SET CMP0053 OLD)

macro(cgv_prepare_resources base outfiles)

	# Add a custom build rule for every file
	foreach (infile ${ARGN})
		_res_prep_command_add("${base}" "${infile}" outfile)
		set(${outfiles} "${${outfiles}}" "${outfile}")
	endforeach()

	# Add the include directory 
	include_directories("${RP_BASE}")
endmacro()


macro(_res_prep_command_add base infile outfile)

	# Generate the output name
	set(RP_BASE "${CMAKE_CURRENT_BINARY_DIR}")

	get_filename_component(RP_ABS "${infile}" ABSOLUTE)
	file(RELATIVE_PATH RP_REL "${base}" "${RP_ABS}")

	if (IS_ABSOLUTE "${RP_REL}")
		message(FATAL_ERROR "The res_prep input file ${infile} was explicitely based to ${${PROJECT_NAME}_RP_BASE} which is not a file base!")
	endif()

	get_filename_component(RP_PATH "${RP_BASE}/${RP_REL}" PATH)		

	get_filename_component(RP_NAME "${infile}" NAME_WE)

	# Create the output directory if it does not exist
	if (NOT EXISTS "${RP_PATH}")
		file(MAKE_DIRECTORY "${RP_PATH}")
	endif()

	get_filename_component(${outfile} "${RP_PATH}/${RP_NAME}.cxx" ABSOLUTE)

	# FIXME: Can we really do this?
	include_directories(
		"${RP_BASE}"
		"${RP_PATH}")

	# Add the build rule
	add_custom_command(OUTPUT ${${outfile}}
		COMMAND ${res_prep_EXECUTABLE}
		ARGS "${CMAKE_CURRENT_SOURCE_DIR}/${infile}" "${${outfile}}"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${${infile}}")
endmacro()

