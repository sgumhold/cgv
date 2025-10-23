
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")

cgv_add_target(rgbd_control
	TYPE plugin
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	SHADER_SOURCES ${SHADERS}
	DEPENDENCIES 
		cgv_utils cgv_type cgv_data cgv_base cgv_signal cgv_reflect cgv_media cgv_os cgv_gui cgv_render cgv_gl cgv_viewer cg_fltk cg_ext crg_stereo_view cmi_io crg_grid glew rgbd_capture rgbd_kinect rgbd_realsense rgbd_kinect_azure point_cloud
	ADDITIONAL_CMDLINE_ARGS
		"config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""
)

install(TARGETS rgbd_control EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
