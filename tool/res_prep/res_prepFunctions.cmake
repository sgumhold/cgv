
function(cgv_prepare_resources base outfiles_var outinclude_var)
    set(RP_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/rp)

    # Add a custom build rule for every file
    foreach (infile ${ARGN})
        cgv_res_prep_command_add("${base}" "${infile}" outfile outinclude)
        list(APPEND o_files ${outfile})
        list(APPEND o_includes ${outinclude})
    endforeach()

    list(APPEND o_includes ${RP_INCLUDE_DIR})

    set(${outfiles_var} ${o_files} PARENT_SCOPE)
    set(${outinclude_var} ${o_includes} PARENT_SCOPE)
endfunction()

function(cgv_res_prep_command_add base infile outfile)
    set(RP_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR})

    get_filename_component(RP_ABS "${infile}" ABSOLUTE)
    file(RELATIVE_PATH RP_REL "${base}" "${RP_ABS}")

    if (IS_ABSOLUTE "${RP_REL}")
        message(FATAL_ERROR "The res_prep input file ${infile} was explicitely based to ${${PROJECT_NAME}_RP_BASE} which is not a file base!")
    endif()

    get_filename_component(RP_PATH "${RP_INCLUDE_DIR}/${RP_REL}" PATH)
    get_filename_component(RP_NAME "${infile}" NAME_WE)

    # Create the output directory if it does not exist
    if (NOT EXISTS "${RP_PATH}")
        file(MAKE_DIRECTORY "${RP_PATH}")
    endif()

    get_filename_component(OUTPUT_FILE "${RP_PATH}/${RP_NAME}.cxx" ABSOLUTE)

    # Add the build rule
    add_custom_command(OUTPUT ${OUTPUT_FILE}
        COMMAND res_prep
        ARGS "${CMAKE_CURRENT_SOURCE_DIR}/${infile}" "${OUTPUT_FILE}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${infile}")
    set(${outfile_var} ${OUTPUT_FILE} PARENT_SCOPE)
    set(${outinclude_var} ${RP_PATH} PARENT_SCOPE)
endfunction()
