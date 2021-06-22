get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

message(STATUS "Importing CGV: ${SELF_DIR}")

if (EXISTS "${SELF_DIR}/CMakeLists.txt")
    add_subdirectory(${SELF_DIR} ${CMAKE_BINARY_DIR}/cgv)
else()

# NOTE: the order of these includes is important!
include(${SELF_DIR}/lib/cgv.cmake)
include(${SELF_DIR}/lib/cgv_3rd.cmake)
include(${SELF_DIR}/lib/cgv_libs.cmake)
include(${SELF_DIR}/lib/cgv_plugins.cmake)
include(${SELF_DIR}/lib/cgv_tools.cmake)
include(${SELF_DIR}/lib/cgv_viewer.cmake)
include(${SELF_DIR}/lib/set_ide_params.cmake)

endif()
