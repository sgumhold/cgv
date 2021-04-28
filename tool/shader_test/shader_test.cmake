function(shader_test base outfiles_var outinclude_var outinstall_var)
    set(ST_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/st")
    file(RELATIVE_PATH ST_INSTALL_POSTFIX ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})
    get_filename_component(ST_BASE "${base}" ABSOLUTE)

    list(APPEND o_includes ${ST_INCLUDE_DIR})

    # Add a custom build rule for every file
    foreach (infile ${ARGN})
        shader_test_command_add("${ST_BASE}" "${infile}" outfile outinclude)
        list(APPEND o_files ${outfile})
        list(APPEND o_includes ${outinclude})
    endforeach ()

    set(${outfiles_var} ${o_files} PARENT_SCOPE)
    set(${outinclude_var} ${o_includes} PARENT_SCOPE)
    set(${outinstall_var} ${ST_INCLUDE_DIR}/${ST_INSTALL_POSTFIX}/. PARENT_SCOPE)
endfunction()

function(shader_test_command_add base infile outfile_var outinclude_var)
    set(ST_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/st")

    get_filename_component(ST_ABS "${infile}" ABSOLUTE)
    file(RELATIVE_PATH ST_REL "${base}" "${ST_ABS}")

    get_filename_component(ST_PATH "${ST_INCLUDE_DIR}/${ST_REL}" PATH)
    get_filename_component(ST_NAME "${infile}" NAME)

    # Create the output directory if it does not exist
    if (NOT EXISTS "${ST_PATH}")
        file(MAKE_DIRECTORY "${ST_PATH}")
    endif ()

    set(INPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${infile}")
    get_filename_component(OUTPUT_FILE "${ST_PATH}/${ST_NAME}.h" ABSOLUTE)
    get_filename_component(PH_SRC_PATH "${infile}" DIRECTORY)

    # Add the build rule
    add_custom_command(OUTPUT ${OUTPUT_FILE}
            COMMAND ${CMAKE_COMMAND} -E env CGV_OPTIONS=SHADER_DEVELOPER $<TARGET_FILE:shader_test>
            ARGS "${INPUT_FILE}" "${OUTPUT_FILE}"
            DEPENDS "${INPUT_FILE}")
    set(${outfile_var} ${OUTPUT_FILE} PARENT_SCOPE)
    set(${outinclude_var} ${PH_PATH} PARENT_SCOPE)
endfunction()
