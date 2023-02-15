
cgv_add_target(vr_draw
	TYPE plugin
	SOURCES vr_draw.cxx
	DEPENDENCIES 
		cgv_utils cgv_type cgv_data cgv_base cgv_math cgv_media cgv_gui cgv_render cgv_gl cg_vr cg_nui
		cgv_viewer cg_fltk crg_grid cg_ext cmi_io openvr_driver crg_vr_view vr_lab vr_emulator crg_vr_wall

	# we're directly using exported symbols from these plugins
	LINKTIME_PLUGIN_DEPENDENCIES crg_vr_view vr_lab

	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)
