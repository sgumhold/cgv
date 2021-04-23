
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cg_gamepad ${SOURCES})
target_compile_definitions(cg_gamepad PRIVATE CG_GAMEPAD_EXPORTS)
target_link_libraries(cg_gamepad
        cgv_utils
        cgv_type
        cgv_reflect
        cgv_data
        cgv_signal
        cgv_os
        cgv_base
        cgv_math
        cgv_media
        cgv_gui
        cgv_reflect_types
        gamepad)

install(TARGETS cg_gamepad EXPORT cg_gamepad DESTINATION ${CGV_LIBS_LIB_DEST})
install(EXPORT cg_gamepad DESTINATION ${CGV_LIBS_LIB_DEST})
