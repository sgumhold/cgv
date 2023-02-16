
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(cg_fltk
	TYPE plugin NO_EXECUTABLE
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES
		fltk cgv_gl cgv_gui cgv_render cgv_media cgv_utils cgv_os cgv_base
		cgv_data cgv_type cgv_signal cgv_reflect

	OVERRIDE_SHARED_EXPORT_DEFINE "CG_FLTK_EXPORTS;CGV_GUI_FLTK_EXPORTS"
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_GUI_FLTK_FORCE_STATIC
)

install(TARGETS cg_fltk EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(TARGETS cg_fltk_static EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
