
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(crg_vr_wall
	TYPE plugin NO_EXECUTABLE
	SOURCES        ${SOURCES}
	HEADERS        ${HEADERS}
	DEPENDENCIES
		vr cgv_type cgv_utils cgv_base cgv_data cgv_signal cgv_reflect cgv_reflect_types cgv_render
		cgv_gl cg_vr cgv_gui cg_ext crg_vr_view
 
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_VR_WALL_EXPORTS
)
