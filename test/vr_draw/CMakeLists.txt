add_library(vr_draw vr_draw.cxx)
target_link_libraries(vr_draw vr_lab)
add_dependencies(vr_draw vr_emulator)
#        cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_render
#        cgv_gui glsu cgv_gl vr cg_vr cg_gamepad gamepad)
