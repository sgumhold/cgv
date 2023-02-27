
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")

cgv_add_target(crg_vr_view
	TYPE plugin NO_EXECUTABLE
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}
	SHADER_SOURCES ${SHADERS}
	DEPENDENCIES
		glsu vr cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_reflect_types cgv_render
		cgv_gui cgv_gl crg_stereo_view_lib cmi_io cg_fltk crg_grid cg_ext cg_vr vr_emulator openvr_driver

	OVERRIDE_SHARED_EXPORT_DEFINE CGV_VR_VIEW_EXPORTS
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_GUI_FORCE_STATIC
)

cgv_add_target(crg_vr_view_lib
	TYPE library
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}
	SHADER_SOURCES ${SHADERS}
	DEPENDENCIES
		glsu vr cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_reflect_types cgv_render
		cgv_gui cgv_gl crg_stereo_view_lib cg_vr vr_emulator openvr_driver

	OVERRIDE_SHARED_EXPORT_DEFINE CGV_VR_VIEW_EXPORTS
	OVERRIDE_FORCE_STATIC_DEFINE  CGV_GUI_FORCE_STATIC

	ADDITIONAL_PRIVATE_DEFINES NO_VR_VIEW_INTERACTOR
)

#target_include_directories(crg_vr_view PUBLIC
#        $<BUILD_INTERFACE:${CGV_DIR}/plugins/crg_stereo_view>
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
#        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS crg_vr_view EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(TARGETS crg_vr_view_lib EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
