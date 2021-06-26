option(BUILD_DOC "Build documentation" ON)

find_package(Doxygen)

if (DOXYGEN_FOUND AND BUILD_DOC)

    add_custom_target(doc
        COMMAND ${CMAKE_COMMAND} -E env CGV_BUILD=${CMAKE_CURRENT_BINARY_DIR} CGV_DIR=${CGV_DIR} ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/cgv.dox
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating Documentation with Doxygen"
        VERBATIM)

elseif (BUILD_DOC)
    message(STATUS "Doxygen needs to be installed to generate the doxygen documentation")
else()
    message(STATUS "Documentation will not be built")
endif ()
