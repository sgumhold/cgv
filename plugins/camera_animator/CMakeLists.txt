
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(camera_animator
	TYPE plugin NO_EXECUTABLE
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}
	DEPENDENCIES
		vr cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_reflect_types cgv_render
		cgv_gl cgv_g2d cgv_app xml cgv_gui cg_ext
 
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_CAMERA_ANIMATOR_EXPORTS
)
