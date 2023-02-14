
cgv_add_target(vr_emulator
	TYPE plugin NO_EXECUTABLE
	SOURCES        vr_emulator.cxx
	HEADERS        vr_emulator.h
	DEPENDENCIES
		glsu vr gamepad cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_render cgv_gui cgv_gl
		cg_vr cg_gamepad
)
install(TARGETS vr_emulator EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
