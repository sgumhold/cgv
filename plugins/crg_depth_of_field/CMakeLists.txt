
cgv_add_target(crg_depth_of_field
	TYPE plugin NO_EXECUTABLE
	SOURCES depth_of_field.cxx
	DEPENDENCIES cgv_base cgv_gl
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_DEPTH_OF_FIELD_EXPORTS
)

install(TARGETS crg_depth_of_field EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
