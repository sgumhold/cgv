
cgv_add_target(cmv_avi
	TYPE plugin NO_EXECUTABLE
	SOURCES avi_video_reader.cxx avi_video_writer.cxx
	DEPENDENCIES Vfw32 cgv_media cgv_base cgv_data cgv_type cgv_reflect cgv_utils
	OVERRIDE_SHARED_EXPORT_DEFINE CGV_MEDIA_VIDEO_WIN_EXPORTS
)
install(TARGETS cmv_avi EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
