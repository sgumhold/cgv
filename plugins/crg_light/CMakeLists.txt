
add_library(crg_light light_interactor.cxx)
target_link_libraries(crg_light cgv_render cgv_gl)
target_compile_definitions(crg_light PRIVATE CGV_RENDER_LIGHT_EXPORTS)

install(TARGETS crg_light EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
