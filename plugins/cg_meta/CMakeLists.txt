
cgv_add_target(cg_meta
	TYPE plugin NO_EXECUTABLE
	SOURCES object_browser.cxx
	DEPENDENCIES cgv_base cgv_gui
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_GUI_META_EXPORTS
)

install(TARGETS cg_meta EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
