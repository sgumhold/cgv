
add_library(vr_test vr_test.cxx)
target_link_libraries(vr_test
        cgv_utils cgv_type cgv_data cgv_base cgv_math
        cgv_media cgv_gui cgv_render glew crg_vr_view)
target_include_directories(vr_test PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<BUILD_INTERFACE:${CGV_DIR}/plugins/crg_vr_view>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
add_dependencies(vr_test
        cgv_viewer

        cmi_io
        cg_fltk crg_grid cg_ext cgv_gl
        crg_vr_wall
        cg_vr vr_emulator openvr_driver)

install(TARGETS vr_test EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
