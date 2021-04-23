get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

message(STATUS "Importing CGV: ${SELF_DIR}")

# NOTE the order of these includes is important!
include(${SELF_DIR}/cgv/cgv_3rd.cmake)
include(${SELF_DIR}/cgv/cgv.cmake)
include(${SELF_DIR}/cgv/cgv_libs.cmake)
