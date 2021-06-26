add_library(vr_lab vr_scene.cxx vr_tool.cxx label_manager.cxx)
target_compile_definitions(vr_lab PRIVATE VR_LAB_EXPORTS)
target_link_libraries(vr_lab cgv_base cgv_gl crg_vr_view rect_pack)
install(TARGETS vr_lab EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
