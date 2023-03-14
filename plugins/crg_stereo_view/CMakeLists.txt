
set(SOURCES stereo_view_interactor.cxx)
set(HEADERS lib_begin.h stereo_view_interactor.h)

cgv_add_target(crg_stereo_view
	TYPE plugin NO_EXECUTABLE
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES
		glsu cgv_render cgv_gui cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_signal cgv_gl

	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_STEREO_VIEW_EXPORTS
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_GUI_FORCE_STATIC
)
cgv_add_target(crg_stereo_view_lib
	TYPE library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES
		glsu cgv_render cgv_gui cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_signal cgv_gl

	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_STEREO_VIEW_EXPORTS
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_GUI_FORCE_STATIC

	ADDITIONAL_PRIVATE_DEFINES NO_STEREO_VIEW_INTERACTOR
)

install(TARGETS crg_stereo_view EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(TARGETS crg_stereo_view_static EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(TARGETS crg_stereo_view_lib EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
