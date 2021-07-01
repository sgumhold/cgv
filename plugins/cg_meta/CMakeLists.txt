
add_library(cg_meta object_browser.cxx)
target_link_libraries(cg_meta cgv_base cgv_gui)
target_compile_definitions(cg_meta PRIVATE CGV_GUI_META_EXPORTS)

install(TARGETS cg_meta EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
