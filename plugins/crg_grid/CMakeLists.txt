
cgv_add_target(crg_grid
	TYPE plugin NO_EXECUTABLE
	SOURCES grid.cxx main.cxx
	DEPENDENCIES
		cgv_render cgv_gui cgv_base cgv_type cgv_reflect cgv_utils cgv_signal cgv_data cgv_gl
	OVERRIDE_SHARED_EXPORT_DEFINE _USRDLL
)

install(TARGETS crg_grid EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
