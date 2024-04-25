
function(shader_test TARGET_NAME outfiles_var outinclude_var outinstall_var shader_reg_inc_filepath_var)
	#cmake_parse_arguments(
	#	PARSE_ARGV 4 CGVARG_ "" "SHADER_REG_INC_FILEPATH_VAR" ""
	#)

	# decide directories
	set(ST_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/st")
	file(RELATIVE_PATH ST_INSTALL_POSTFIX ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

	# add a custom build rule for every file and assemble the contents of ..._shader_inc.h
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
		list(APPEND OFILES ${OFILE_FULL_PATH})

		# add corresponding entry to ..._shader_inc.h contents
		set(CGV_SHADER_INC_CONTENT
			"${CGV_SHADER_INC_CONTENT}\n#include <${OFILE}>\ncgv::base::resource_string_registration ${CPP_NAME}_reg(\"${IFILE}\", ${CPP_NAME});\n"
		)

		# add the build rule
		cgv_stringify_generatortime_list(CGV_OPTIONS_STRING "${CGV_OPTIONS}")
		add_custom_command(
			OUTPUT ${OFILE_FULL_PATH}
			COMMAND ${CMAKE_COMMAND} -E env CGV_DIR="${CGV_DIR}" CGV_OPTIONS="${CGV_OPTIONS_STRING}" $<TARGET_FILE:shader_test>
			ARGS "${IFILE_FULL_PATH}" "${OFILE_FULL_PATH}"
			DEPENDS "${IFILE_FULL_PATH}" shader_test
		)
	endforeach()

	# generate the ..._shader_inc.h file
	set(SHADER_INC_FILEPATH "${ST_INCLUDE_DIR}/${TARGET_NAME}_shader_inc.h")
	configure_file("${CGV_DIR}/make/cmake/shader_inc.h.in" "${SHADER_INC_FILEPATH}" @ONLY)

	# propagate results upwards
	set(${outfiles_var} ${OFILES} PARENT_SCOPE)
	set(${outinclude_var} ${ST_INCLUDE_DIR} PARENT_SCOPE)
	set(${outinstall_var} ${ST_INCLUDE_DIR}/${ST_INSTALL_POSTFIX}/. PARENT_SCOPE)
	if (shader_reg_inc_filepath_var)
		set(${shader_reg_inc_filepath_var} ${SHADER_INC_FILEPATH} PARENT_SCOPE)
	endif()
endfunction()
