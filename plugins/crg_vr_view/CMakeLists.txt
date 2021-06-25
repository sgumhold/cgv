
add_library(crg_vr_view vr_render_helpers.cxx vr_view_interactor.cxx)
target_compile_definitions(crg_vr_view PRIVATE CGV_VR_VIEW_EXPORTS)
target_link_libraries(crg_vr_view
        cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_reflect_types
        cgv_render cgv_gui cg_ext glsu cgv_gl crg_stereo_view_lib vr cg_vr)
target_include_directories(crg_vr_view PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}/plugins/crg_stereo_view>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
add_dependencies(crg_vr_view
        cgv_viewer

        cmi_io
        cg_fltk crg_grid cg_ext cgv_gl
        cg_vr vr_emulator openvr_driver)

install(TARGETS crg_vr_view EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
