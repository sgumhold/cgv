
function(shader_test TARGET_NAME base outfiles_var outinclude_var outinstall_var)
	set(ST_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/st")
	file(RELATIVE_PATH ST_INSTALL_POSTFIX ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
	#message("=========================================================================================")
	#message(" invoked shader_test(")
	#message("	   TARGET_NAME = ${TARGET_NAME}")
	#message("	          base = ${base}")
	#message("	  outfiles_var = ${outfiles_var}")
	#message("	outinclude_var = ${outinclude_var}")
	#message("	outinstall_var = ${outinstall_var}")
	#message(" )")
	#message(" target ST_INCLUDE_DIR: ${ST_INCLUDE_DIR}")
	#message(" -------------------------------------------------------------------------------------")
	get_filename_component(ST_BASE "${base}" ABSOLUTE)

	#list(APPEND o_includes ${ST_INCLUDE_DIR})

	# add a custom build rule for every file and assemble the contents of ..._shader_inc.h
	set(CGV_SHADER_INC_CONTENT "")
	foreach (IFILE_ARG ${ARGN})
		# determine filenames and derived strings
		get_filename_component(IFILE_FULL_PATH ${IFILE_ARG} ABSOLUTE) # <-- evaluates with respect to CMAKE_CURRENT_SOURCE_DIR
		get_filename_component(IFILE ${IFILE_ARG} NAME)
		get_filename_component(IFILE_WITHOUT_EXT ${IFILE} NAME_WLE)
		get_filename_component(IFILE_EXT ${IFILE} LAST_EXT)
		string(REGEX REPLACE ".(.*)" "\\1" IFILE_EXT_WITHOUT_DOT ${IFILE_EXT})
		set(CPP_NAME "${IFILE_WITHOUT_EXT}_${IFILE_EXT_WITHOUT_DOT}")
		set(OFILE "${IFILE}.log")
		set(OFILE_FULL_PATH "${ST_INCLUDE_DIR}/${OFILE}")
		#message("shader_test: IFILE_ARG:'${IFILE_ARG}' ('${IFILE}') --> IFILE_FULL_PATH:'${IFILE_FULL_PATH}'")
		#message("             OFILE:'${OFILE}' --> OFILE_FULL_PATH:'${OFILE_FULL_PATH}'")
		#message("             IFILE_WITHOUT_EXT:'${IFILE_WITHOUT_EXT}' IFILE_EXT:'${IFILE_EXT}'")
		#message("             IFILE_EXT_WITHOUT_DOT:'${IFILE_EXT_WITHOUT_DOT}' --> CPP_NAME:'${CPP_NAME}'")
		list(APPEND OFILES ${OFILE_FULL_PATH})

		# add corresponding entry to ..._shader_inc.h contents
		set(
			CGV_SHADER_INC_CONTENT
			"${CGV_SHADER_INC_CONTENT}\n#include <${OFILE}>\ncgv::base::resource_string_registration ${CPP_NAME}_reg(\"${IFILE}\", ${CPP_NAME});\n"
		)

		# add the build rule
		add_custom_command(OUTPUT ${OFILE_FULL_PATH}
			COMMAND ${CMAKE_COMMAND} -E env CGV_DIR=${CGV_DIR} CGV_OPTIONS=${CGV_OPTIONS} $<TARGET_FILE:shader_test>
			ARGS "${IFILE_FULL_PATH}" "${OFILE_FULL_PATH}"
			DEPENDS "${IFILE_FULL_PATH}")
	endforeach()

	# generate the ..._shader_inc.h file
	#message(" + RESULTING CONTENT:")
	#message("${CGV_SHADER_INC_CONTENT}")
	configure_file(
		"${CGV_DIR}/make/cmake/shader_inc.h.in" "${ST_INCLUDE_DIR}/${TARGET_NAME}_shader_inc.h" @ONLY
	)

	# propagate results upwards
	set(${outfiles_var} ${OFILES} PARENT_SCOPE)
	set(${outinclude_var} ${ST_INCLUDE_DIR} PARENT_SCOPE)
	set(${outinstall_var} ${ST_INCLUDE_DIR}/${ST_INSTALL_POSTFIX}/. PARENT_SCOPE)
endfunction()

#function(shader_test_command_add base infile outfile_var outinclude_var)
#	set(ST_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/st")
#
#	get_filename_component(ST_ABS "${infile}" ABSOLUTE)
#	file(RELATIVE_PATH ST_REL "${base}" "${ST_ABS}")
#
#	get_filename_component(ST_PATH "${ST_INCLUDE_DIR}/${ST_REL}" PATH)
#	get_filename_component(ST_NAME "${infile}" NAME)
#
#	# Create the output directory if it does not exist
#	if (NOT EXISTS "${ST_PATH}")
#		file(MAKE_DIRECTORY "${ST_PATH}")
#	endif ()
#
#	set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${infile}")
#	get_filename_component(OUTPUT_FILE "${ST_PATH}/${ST_NAME}.log" ABSOLUTE)
#	get_filename_component(PH_SRC_PATH "${infile}" DIRECTORY)
#
#	# Add the build rule
#	add_custom_command(OUTPUT ${OUTPUT_FILE}
#		COMMAND ${CMAKE_COMMAND} -E env CGV_DIR=${CGV_DIR} CGV_OPTIONS=${CGV_OPTIONS} $<TARGET_FILE:shader_test>
#		ARGS "${INPUT_FILE}" "${OUTPUT_FILE}"
#		DEPENDS "${INPUT_FILE}")
#	set(${outfile_var} ${OUTPUT_FILE} PARENT_SCOPE)
#	set(${outinclude_var} ${PH_PATH} PARENT_SCOPE)
#endfunction()
