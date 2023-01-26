
# add a target to the build system that is decorated with several CGV Framework-specific custom properties allowing for
# advanced features like automatic launch/debug command line generation, resource embedding for single-executable builds etc.
function(cgv_add_target NAME)
    cmake_parse_arguments(
        PARSE_ARGV 1 CGVARG_
        "" "TYPE"
        "SOURCES;PPP_SOURCES;HEADERS;RESOURCES;AUDIO_RESOURCES;SHADER_SOURCES;DEPENDENCIES"
    )

    # prelude
    set(IS_LIBRARY FALSE)
    set(IS_CORELIB FALSE)
    set(IS_PLUGIN FALSE)
    if (NOT CGVARG__TYPE)
        message(FATAL_ERROR "cgv_add_target(): no type specified for CGV build target '${NAME}'!")
    elseif (CGVARG__TYPE STREQUAL "corelib")
        set(IS_LIBRARY TRUE)
        set(IS_CORELIB TRUE)
        string(SUBSTRING ${NAME} 4 -1 HEADER_INSTALL_DIR)
        set(HEADER_INSTALL_DIR ${CGV_INCLUDE_DEST}/${HEADER_INSTALL_DIR})
        set(EXPORT_TARGET cgv)
    else()
        if (CGVARG__TYPE STREQUAL "library")
            set(IS_LIBRARY TRUE)
        elseif (CGVARG__TYPE MATCHES "plugin$")
            set(IS_PLUGIN TRUE)
        endif()
        set(HEADER_INSTALL_DIR ${CGV_LIBS_INCLUDE_DEST}/${NAME})
        set(EXPORT_TARGET cgv_libs)
    endif()

    # process collected files to be attached to the target (and attach the processing results if any)
    # - PPP generated sources/headers/generic files
    if (CGVARG__PPP_SOURCES)
        ppp_compile("${CGV_DIR}"
                PPP_FILES
                PPP_INCLUDES
                PPP_INSTALL_DIR
                ${CGVARG__PPP_SOURCES})
        install(DIRECTORY ${PPP_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
    endif()
    # - shader files
    set(SHADER_PATHS "")
    if (CGVARG__SHADER_SOURCES)
        foreach (SHADER_SOURCE ${CGVARG__SHADER_SOURCES})
            cmake_path(
                ABSOLUTE_PATH SHADER_SOURCE BASE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} NORMALIZE
                OUTPUT_VARIABLE SHADER_SOURCE_ABSOLUTE_PATH)
            cmake_path(GET SHADER_SOURCE_ABSOLUTE_PATH PARENT_PATH SHADER_SOURCE_PARENT_PATH)
            list(APPEND SHADER_PATHS ${SHADER_SOURCE_PARENT_PATH})
        endforeach()
        list(REMOVE_DUPLICATES SHADER_PATHS)

        shader_test("${CGV_DIR}"
                ST_FILES
                ST_INCLUDES
                ST_INSTALL_DIR
                ${CGVARG__SHADER_SOURCES})

        install(DIRECTORY ${ST_INSTALL_DIR} DESTINATION ${HEADER_INSTALL_DIR} FILES_MATCHING PATTERN "*.h")
    endif ()
    # - generic resources (e.g. images, icons)
    if (CGVARG__RESOURCES)
        cgv_prepare_resources(${CMAKE_SOURCE_DIR} RESOURCE_SRCFILES ${CGVARG__RESOURCES})
    endif()
    # - audio resources
    if(CGVARG__AUDIO_RESOURCES AND CGV_BUILD_WITH_AUDIO)
        cgv_prepare_resources(${CMAKE_SOURCE_DIR} AUDIO_RESOURCE_SRCFILES ${CGVARG__AUDIO_RESOURCES})
    endif()

    string(TOUPPER ${NAME} NAME_UPPER)
    set(NAME_STATIC ${NAME}_static)
    set(ALL_SOURCES
        ${CGVARG__SOURCES} ${PPP_FILES} ${CGVARG__PPP_SOURCES} ${CGVARG__HEADERS} ${ST_FILES} ${SHADERS}
        ${RESOURCE_SRCFILES} ${CGVARG__RESOURCES} ${AUDIO_RESOURCE_SRCFILES} ${CGVARG__AUDIO_RESOURCES} 
    )

    # Shared Library
    add_library(${NAME} SHARED ${ALL_SOURCES})
    set_target_properties(${NAME} PROPERTIES CGVPROP_TYPE "${CGVARG__TYPE}")
    set_target_properties(${NAME} PROPERTIES CGVPROP_SHADERPATHS "${SHADER_PATHS}")
    target_compile_definitions(${NAME} PRIVATE ${NAME_UPPER}_EXPORTS)
    foreach (DEPENDENCY ${ARGS_DEPENDENCIES})
        target_link_libraries(${NAME} PUBLIC ${DEPENDENCY})
        if (IS_PLUGIN)
            get_target_property(DEPENDENCY_TYPE ${DEPENDENCY} CGVPROP_TYPE)
            if (DEPENDENCY_TYPE MATCHES "plugin$")
                set_target_properties(${NAME} PROPERTIES CGVPROP_REQUESTED_PLUGINS ${DEPENDENCY})
            endif()
        endif()
    endforeach()

    target_include_directories(${NAME} PUBLIC
            $<BUILD_INTERFACE:${CGV_DIR}>
            "$<BUILD_INTERFACE:${PPP_INCLUDES}>"
            "$<BUILD_INTERFACE:${ST_INCLUDES}>")
    if (IS_CORELIB)
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

    # /BEGIN DEBUG OUTPUT
    get_target_property(PROPVAL_TYPE ${NAME} CGVPROP_TYPE)
    message("CGV TARGET: ${NAME} (${PROPVAL_TYPE})")
    get_target_property(PROPVAL_SP ${NAME} CGVPROP_SHADERPATHS)
    if (PROPVAL_SP)
        message(" [shader path]  ${PROPVAL_SP}")
    else()
        message(" [shader path]  <none>")
    endif()
    if (IS_PLUGIN)
        get_target_property(PROPVAL_RP ${NAME} CGVPROP_REQUESTED_PLUGINS)
        if (PROPVAL_RP)
            message(" [req.plugins]  ${PROPVAL_RP}")
        else()
            message(" [req.plugins]  <none>")
        endif()
    endif()
    get_target_property(PROPVAL_CDEFS ${NAME} COMPILE_DEFINITIONS)
    message(" [  defines  ]  ${PROPVAL_CDEFS}")
    # /END DEBUG OUTPUT

    install(TARGETS ${NAME_STATIC} EXPORT ${EXPORT_TARGET} DESTINATION ${CGV_BIN_DEST})
endfunction()


# DEPRECATED - kept for backwards compatibility. To-be-removed!
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

    set(LIB_SHADER_PATHS "")
    if (ARGS_SHADER_SOURCES)
        foreach (SHADER_SOURCE ${ARGS_SHADER_SOURCES})
            cmake_path(
                ABSOLUTE_PATH SHADER_SOURCE BASE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} NORMALIZE
                OUTPUT_VARIABLE SHADER_SOURCE_ABSOLUTE_PATH)
            cmake_path(GET SHADER_SOURCE_ABSOLUTE_PATH PARENT_PATH SHADER_SOURCE_PARENT_PATH)
            list(APPEND LIB_SHADER_PATHS ${SHADER_SOURCE_PARENT_PATH})
        endforeach()
        list(REMOVE_DUPLICATES LIB_SHADER_PATHS)
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
    if (ARGS_CORE_LIB)
        set_target_properties(${NAME} PROPERTIES CGVPROP_TYPE "corelib")
    else()
        set_target_properties(${NAME} PROPERTIES CGVPROP_TYPE "library")
    endif()
    set_target_properties(${NAME} PROPERTIES CGVPROP_SHADERPATHS "${LIB_SHADER_PATHS}")
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
