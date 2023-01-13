add_library(crg_bg background.cxx)
target_link_libraries(crg_bg cgv_render cgv_gl)
target_compile_definitions(crg_bg PRIVATE CGV_RENDER_BACKGROUND_EXPORTS)
install(TARGETS crg_bg EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
