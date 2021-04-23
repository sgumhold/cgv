
get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

# NOTE the order of these includes is important!
include(${SELF_DIR}/cgv_math.cmake)
include(${SELF_DIR}/cgv_utils.cmake)
include(${SELF_DIR}/cgv_type.cmake)
include(${SELF_DIR}/cgv_signal.cmake)
include(${SELF_DIR}/cgv_reflect.cmake)
include(${SELF_DIR}/cgv_data.cmake)
include(${SELF_DIR}/cgv_base.cmake)
include(${SELF_DIR}/cgv_media.cmake)
include(${SELF_DIR}/cgv_gui.cmake)
include(${SELF_DIR}/cgv_os.cmake)
include(${SELF_DIR}/cgv_ppp.cmake)
include(${SELF_DIR}/cgv_render.cmake)
