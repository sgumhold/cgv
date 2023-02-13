
cgv_add_target(crg_antialias
	TYPE plugin NO_EXECUTABLE
	SOURCES antialias.cxx
	DEPENDENCIES cgv_base cgv_gl
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_ANTIALIAS_EXPORTS
)

install(TARGETS crg_antialias EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
