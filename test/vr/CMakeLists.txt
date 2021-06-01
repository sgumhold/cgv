add_library(vr_emulator vr_emulator.cxx)
target_compile_definitions(vr_emulator PRIVATE VR_EMULATOR_EXPORTS)
target_link_libraries(vr_emulator
        cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_render
        cgv_gui glsu cgv_gl vr cg_vr cg_gamepad gamepad)
