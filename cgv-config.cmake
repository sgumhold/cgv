get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

message(STATUS "Importing CGV: ${SELF_DIR}")

if (EXISTS "${SELF_DIR}/CMakeLists.txt")
    add_subdirectory(${SELF_DIR} ${CMAKE_BINARY_DIR}/cgv EXCLUDE_FROM_ALL)
else()

# NOTE: the order of these includes is important!
include(${SELF_DIR}/bin/cgv.cmake)
include(${SELF_DIR}/bin/cgv_3rd.cmake)
include(${SELF_DIR}/bin/cgv_libs.cmake)
include(${SELF_DIR}/bin/cgv_plugins.cmake)
include(${SELF_DIR}/bin/cgv_tools.cmake)
include(${SELF_DIR}/bin/cgv_viewer.cmake)
include(${SELF_DIR}/bin/set_plugin_execution_params.cmake)

endif()
