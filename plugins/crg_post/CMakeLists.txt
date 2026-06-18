
cgv_add_target(crg_post
    TYPE plugin NO_EXECUTABLE
	SOURCES post_processor.cxx
	DEPENDENCIES cgv_render cgv_gl cgv_post cgv_gui
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_POSTPROC_EXPORTS
)

install(TARGETS crg_post EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
