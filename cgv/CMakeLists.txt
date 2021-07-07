
function(cgv_create_core_lib NAME)
    cmake_parse_arguments(ARGS "" "" "SOURCES;PPP_FILES;PPP_INCLUDES;DEPENDENCIES" ${ARGN})

    #    message(STATUS "${ARGS_SOURCES}")
    #    message(STATUS "${ARGS_PPP_FILES}")
    #    message(STATUS "${CMAKE_DL_LIBS}")

    string(TOUPPER ${NAME} NAME_UPPER)
    set(NAME_STATIC ${NAME}_static)

    # Shared Library
    add_library(${NAME} SHARED ${ARGS_SOURCES} ${ARGS_PPP_FILES})
    target_compile_definitions(${NAME} PRIVATE ${NAME_UPPER}_EXPORTS)
    foreach (DEPENDENCY ${ARGS_DEPENDENCIES})
        target_link_libraries(${NAME} PUBLIC ${DEPENDENCY})
    endforeach ()

    target_include_directories(${NAME} PUBLIC
            $<BUILD_INTERFACE:${CGV_DIR}>
            "$<BUILD_INTERFACE:${ARGS_PPP_INCLUDES}>"
            $<INSTALL_INTERFACE:include>)

    install(TARGETS ${NAME} EXPORT cgv DESTINATION ${CGV_BIN_DEST})

    # Static Library
    add_library(${NAME_STATIC} STATIC ${ARGS_SOURCES} ${ARGS_PPP_FILES})
    target_compile_definitions(${NAME_STATIC} PRIVATE CGV_FORCE_STATIC)
    foreach (DEPENDENCY ${ARGS_DEPENDENCIES})
        if (${DEPENDENCY} STREQUAL "${CMAKE_DL_LIBS}")
            continue()
        endif ()
        target_link_libraries(${NAME_STATIC} PUBLIC ${DEPENDENCY}_static)
    endforeach ()
    target_include_directories(${NAME_STATIC} PUBLIC
            $<BUILD_INTERFACE:${CGV_DIR}>
            "$<BUILD_INTERFACE:${ARGS_PPP_INCLUDES}>"
            $<INSTALL_INTERFACE:include>)

    install(TARGETS ${NAME_STATIC} EXPORT cgv DESTINATION ${CGV_BIN_DEST})
endfunction()

add_subdirectory(base)
add_subdirectory(data)
add_subdirectory(gui)
add_subdirectory(math)
add_subdirectory(media)
add_subdirectory(os)
add_subdirectory(ppp)
add_subdirectory(reflect)
add_subdirectory(render)
add_subdirectory(signal)
add_subdirectory(type)
add_subdirectory(utils)

install(EXPORT cgv DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY . DESTINATION ${CGV_INCLUDE_DEST} FILES_MATCHING PATTERN "*.h")
# TODO find out why there are *.hh files (maybe rename them to *.h)
install(DIRECTORY . DESTINATION ${CGV_INCLUDE_DEST} FILES_MATCHING PATTERN "*.hh")
