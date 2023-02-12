
cgv_add_target(crg_light
	TYPE plugin NO_EXECUTABLE
	SOURCES light_interactor.cxx
	DEPENDENCIES cgv_render cgv_gl
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_LIGHT_EXPORTS
)

install(TARGETS crg_light EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
