add_library(vr_lab vr_scene.cxx vr_tool.cxx)
target_compile_definitions(vr_lab PRIVATE VR_LAB_EXPORTS)
target_link_libraries(vr_lab cgv_base cgv_gl cgv_proc cg_nui crg_vr_view)
install(TARGETS vr_lab EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
