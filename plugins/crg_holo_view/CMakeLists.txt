
set(SOURCES holo_view_interactor.cxx)
set(HEADERS lib_begin.h holo_view_interactor.h)
set(SHADERS
	glsl/finish.glfs
	glsl/quilt_finish.glfs
	glsl/quilt_finish.glpr
	glsl/volume_finish.glfs
	glsl/volume_finish.glpr
)

cgv_add_target(crg_holo_view
	TYPE plugin NO_EXECUTABLE
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}
	SHADER_SOURCES ${SHADERS}
	DEPENDENCIES
		glsu cgv_render cgv_gui cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_signal cgv_gl

	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_HOLO_VIEW_EXPORTS
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_GUI_FORCE_STATIC
)
cgv_add_target(crg_holo_view_lib
	TYPE library
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}
	SHADER_SOURCES ${SHADERS}
	DEPENDENCIES
		glsu cgv_render cgv_gui cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_signal cgv_gl

	OVERRIDE_SHARED_EXPORT_DEFINE CGV_RENDER_HOLO_VIEW_EXPORTS
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_GUI_FORCE_STATIC

	ADDITIONAL_PRIVATE_DEFINES NO_HOLO_VIEW_INTERACTOR
)

install(TARGETS crg_holo_view EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(TARGETS crg_holo_view_static EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(TARGETS crg_holo_view_lib EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
