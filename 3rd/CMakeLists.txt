# TODO old-style targets are temporarily decorated with the CGV-defined type property, remove when
# converting them to the new system
add_subdirectory(ANN)
set_target_properties(ann PROPERTIES CGVPROP_TYPE "library")
set_target_properties(annf PROPERTIES CGVPROP_TYPE "library")
add_subdirectory(fltk)
set_target_properties(fltk PROPERTIES CGVPROP_TYPE "library")
add_subdirectory(glew)
set_target_properties(glew PROPERTIES CGVPROP_TYPE "library")
add_subdirectory(googletest)
# no cgv target metadata decoration since googletest is an externally defined, imported CMake target
add_subdirectory(jpeg)
set_target_properties(jpeg PROPERTIES CGVPROP_TYPE "library")
add_subdirectory(json)
set_target_properties(json PROPERTIES CGVPROP_TYPE "library")
set_target_properties(json PROPERTIES CGVPROP_NO_WHOLE_ARCH TRUE) # <-- FIXME: UGLY WORKAROUND HACK. CONVERT TO CMAKE OBJECT LIBRARIES TO MAKE THIS UNECESSARY
add_subdirectory(libtorch)
# no cgv target metadata decoration since the CMake build system currently does not offer PyTorch integration
add_subdirectory(openvr)
set_target_properties(openvr PROPERTIES CGVPROP_TYPE "library")
add_subdirectory(png)
set_target_properties(png PROPERTIES CGVPROP_TYPE "library")
add_subdirectory(tiff)
set_target_properties(tif PROPERTIES CGVPROP_TYPE "library")
add_subdirectory(zlib)
set_target_properties(zlib PROPERTIES CGVPROP_TYPE "library")
add_subdirectory(screen_capture_lite)
set_target_properties(screen_capture_lite PROPERTIES CGVPROP_TYPE "library")
if(CGV_BUILD_WITH_AUDIO)
	add_subdirectory(openal-soft)
	add_subdirectory(sndfile)
	# no cgv target metadata decoration since openal-soft and sndfile are both externally defined, imported CMake targets
endif()

if(WIN32)
	# FIXME webserver can only be built on Windows
	add_subdirectory(webserver)
endif()

install(EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
