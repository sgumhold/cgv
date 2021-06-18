
add_library(crg_vr_wall vr_wall.cxx vr_wall_kit.cxx)
target_compile_definitions(crg_vr_wall PRIVATE CGV_VR_WALL_EXPORTS)
target_link_libraries(crg_vr_wall
        cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_reflect_types
        cgv_render cgv_gui cg_ext cgv_gl vr cg_vr)
target_include_directories(crg_vr_wall PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}/libs/vr>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
