
add_library(crg_antialias antialias.cxx)
target_link_libraries(crg_antialias cgv_base cgv_gl)
target_compile_definitions(crg_antialias PRIVATE CGV_RENDER_ANTIALIAS_EXPORTS)

install(TARGETS crg_antialias EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
