
add_subdirectory(ANN)
add_subdirectory(fltk)
add_subdirectory(glew)
add_subdirectory(googletest)
add_subdirectory(jpeg)
add_subdirectory(json)
add_subdirectory(libtorch)
add_subdirectory(openvr)
add_subdirectory(png)
add_subdirectory(tiff)
add_subdirectory(zlib)
add_subdirectory(screen_capture_lite)
if(BUILD_WITH_AUDIO)
	add_subdirectory(openal-soft)
	add_subdirectory(sndfile)
endif()

if(WIN32)
	# FIXME webserver can only be built on Windows
	add_subdirectory(webserver)
endif()

install(EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
