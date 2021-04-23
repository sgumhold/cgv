
add_library(cmv_avi avi_video_reader.cxx avi_video_writer.cxx)
target_link_libraries(cmv_avi cgv_media cgv_base cgv_data cgv_type cgv_reflect cgv_utils)
target_compile_definitions(cmv_avi PRIVATE CGV_MEDIA_VIDEO_WIN_EXPORTS)

if (WIN32)
    target_link_libraries(cmv_avi Vfw32)
endif ()

install(TARGETS cmv_avi EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
