# Set output base
set(PH_BASE "${CMAKE_BINARY_DIR}/@BUILD_BASE@/@INSTALL_HEADER_PATH@")


macro(ppp_compile base outfiles)

	get_filename_component(PPP_BASE "${base}" ABSOLUTE)

	# Add a custom build rule for every file
	foreach (infile ${ARGN})
		ppp_command_add("${PPP_BASE}" "${infile}" outfile)
		set(${outfiles} "${${outfiles}}" "${outfile}")
	endforeach()

	# Add the include directory 
	include_directories("${PH_BASE}")
endmacro()


macro(ppp_command_add base infile outfile)

# Set PPP working files
# These files are implicit dependencies and will be checked
# for the data. Specify the additional files that are used by the ppp
#	set(PPP_WORK_FILES
#		file1.ppp
#		file2.ppp)	

	# Generate the output name

	get_filename_component(PH_ABS "${infile}" ABSOLUTE)
	file(RELATIVE_PATH PH_REL "${base}" "${PH_ABS}")

	if (IS_ABSOLUTE "${PH_REL}")
		message(FATAL_ERROR "The ppp input file ${infile} was explicitely based to ${${PROJECT_NAME}_PPP_BASE} which is not a file base!")
	endif()

	get_filename_component(PH_PATH "${PH_BASE}/${PH_REL}" PATH)		

	get_filename_component(PH_NAME "${infile}" NAME_WE)

	# Create the output directory if it does not exist
	if (NOT EXISTS "${PH_PATH}")
		file(MAKE_DIRECTORY "${PH_PATH}")
	endif()

	get_filename_component(${outfile} "${PH_PATH}/${PH_NAME}.h" ABSOLUTE)

	# FIXME: Can we really do this?
	include_directories(
		"${PH_BASE}"
		"${PH_PATH}")

	# Add the build rule
	add_custom_command(OUTPUT ${${outfile}}
		COMMAND ppp
		ARGS "-CGV_DIR=${CGV_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${infile}" "${${outfile}}"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${${infile}}"
		IMPLICIT_DEPENDS ${PPP_WORK_FILES})
endmacro()
