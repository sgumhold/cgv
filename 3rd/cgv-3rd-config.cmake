get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

message(STATUS "Importing CGV 3rd-party dependencies: ${SELF_DIR}")

include(${SELF_DIR}/json.cmake)
include(${SELF_DIR}/glew.cmake)
