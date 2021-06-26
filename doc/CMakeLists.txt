
message(STATUS "Looking for Doxygen")

find_package(Doxygen)

if (DOXYGEN_FOUND)
    message(STATUS "Looking for Doxygen - found")

    add_custom_target(doc
        COMMAND ${CMAKE_COMMAND} -E env CGV_BUILD=${CMAKE_CURRENT_BINARY_DIR} CGV_DIR=${CGV_DIR} ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cgv.dox
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating Documentation with Doxygen"
        VERBATIM)

else ()
    message(STATUS "Looking for Doxygen - not found")
endif ()
