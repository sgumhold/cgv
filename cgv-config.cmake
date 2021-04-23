get_filename_component(ROOT_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

message(STATUS "Importing CGV: ${ROOT_DIR}")

# NOTE the order of these includes is important!
include(${ROOT_DIR}/cgv/cgv-3rd-config.cmake)
include(${ROOT_DIR}/cgv/cgv-config.cmake)
include(${ROOT_DIR}/cgv/cgv-libs-config.cmake)
