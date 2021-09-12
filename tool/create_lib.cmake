function(cgv_create_lib NAME)
    cmake_parse_arguments(ARGS "CORE_LIB" "" "SOURCES;PPP_SOURCES;SHADER_SOURCES;DEPENDENCIES" ${ARGN})

    if (ARGS_CORE_LIB)
        string(SUBSTRING ${NAME} 4 -1 HEADER_INSTALL_DIR)
        set(HEADER_INSTALL_DIR ${CGV_INCLUDE_DEST}/${HEADER_INSTALL_DIR})

        set(EXPORT_TARGET cgv)
    else ()
        set(HEADER_INSTALL_DIR ${CGV_LIBS_INCLUDE_DEST}/${NAME})

        set(EXPORT_TARGET cgv_libs)
    endif ()

    if (ARGS_PPP_SOURCES)
        ppp_compile("${CGV_DIR}"
                PPP_FILES
                PPP_INCLUDES
                PPP_INSTALL_DIR
                ${ARGS_PPP_SOURCES})

        install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
    endif ()

    if (ARGS_SHADER_SOURCES)
        shader_test("${CGV_DIR}"
                ST_FILES
                ST_INCLUDES
                ST_INSTALL_DIR
                ${ARGS_SHADER_SOURCES})

        install(DIRECTORY ${ST_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
    endif ()

    string(TOUPPER ${NAME} NAME_UPPER)
    set(NAME_STATIC ${NAME}_static)
    set(ALL_SOURCES ${ARGS_SOURCES} ${PPP_FILES} ${ST_FILES} ${SHADERS})

    # Shared Library
    add_library(${NAME} SHARED ${ALL_SOURCES})
    target_compile_definitions(${NAME} PRIVATE ${NAME_UPPER}_EXPORTS)
    foreach (DEPENDENCY ${ARGS_DEPENDENCIES})
        target_link_libraries(${NAME} PUBLIC ${DEPENDENCY})
    endforeach ()

    target_include_directories(${NAME} PUBLIC
            $<BUILD_INTERFACE:${CGV_DIR}>
            "$<BUILD_INTERFACE:${PPP_INCLUDES}>"
            "$<BUILD_INTERFACE:${ST_INCLUDES}>")
    if (ARGS_CORE_LIB)
        target_include_directories(${NAME} PUBLIC $<INSTALL_INTERFACE:include>)
    else ()
        target_include_directories(${NAME} PUBLIC
                $<BUILD_INTERFACE:${CGV_DIR}/libs>
                $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
    endif ()

    install(TARGETS ${NAME} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})

    # Static Library
    add_library(${NAME_STATIC} STATIC ${ALL_SOURCES})
    target_compile_definitions(${NAME_STATIC} PUBLIC CGV_FORCE_STATIC)
    foreach (DEPENDENCY ${ARGS_DEPENDENCIES})
        if (${DEPENDENCY} STREQUAL "${CMAKE_DL_LIBS}")
            continue()
        endif ()
        target_link_libraries(${NAME_STATIC} PUBLIC ${DEPENDENCY}_static)
    endforeach ()

    target_include_directories(${NAME_STATIC} PUBLIC
            $<BUILD_INTERFACE:${CGV_DIR}>
            "$<BUILD_INTERFACE:${PPP_INCLUDES}>"
            "$<BUILD_INTERFACE:${ST_INCLUDES}>"
            $<INSTALL_INTERFACE:include>)
    if (NOT ARGS_CORE_LIB)
        target_include_directories(${NAME_STATIC} PUBLIC $<BUILD_INTERFACE:${CGV_DIR}/libs>)
    endif ()

    install(TARGETS ${NAME_STATIC} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
endfunction()
