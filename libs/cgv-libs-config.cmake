get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

message(STATUS "Importing CGV libs: ${SELF_DIR}")

include(${SELF_DIR}/gamepad.cmake)
include(${SELF_DIR}/cgv_reflect_types.cmake)
include(${SELF_DIR}/cg_gamepad.cmake)
include(${SELF_DIR}/cgv_gl.cmake)