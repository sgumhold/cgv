# TODO old-style targets are temporarily decorated with the CGV-defined type property and
# IDE project folder, remove when converting them to the new system
add_subdirectory(ANN)
set_target_properties(ann PROPERTIES
	CGVPROP_TYPE "library"
	FOLDER       "3rd"
	CXX_STANDARD "14"
)
set_target_properties(annf PROPERTIES
	CGVPROP_TYPE "library"
	FOLDER       "3rd"
	CXX_STANDARD "14"
)
set_target_properties(ann_static PROPERTIES
	FOLDER       "_obj"
	CXX_STANDARD "14"
)
set_target_properties(annf_static PROPERTIES
	FOLDER       "_obj"
	CXX_STANDARD "14"
)

add_subdirectory(fltk)
set_target_properties(fltk PROPERTIES CGVPROP_TYPE "library")
set_target_properties(fltk PROPERTIES FOLDER "3rd")
set_target_properties(fltk_static PROPERTIES FOLDER "_obj")
add_subdirectory(glew)
set_target_properties(glew PROPERTIES CGVPROP_TYPE "library")
set_target_properties(glew PROPERTIES FOLDER "3rd")
set_target_properties(glew_static PROPERTIES FOLDER "_obj")
add_subdirectory(jpeg)
set_target_properties(jpeg PROPERTIES CGVPROP_TYPE "library")
set_target_properties(jpeg PROPERTIES FOLDER "3rd")
set_target_properties(jpeg_static PROPERTIES FOLDER "_obj")
add_subdirectory(json)
set_target_properties(json PROPERTIES CGVPROP_TYPE "library")
set_target_properties(json PROPERTIES FOLDER "3rd")
set_target_properties(json_static PROPERTIES FOLDER "_obj")
add_subdirectory(libtorch)
add_subdirectory(openvr)
set_target_properties(openvr PROPERTIES CGVPROP_TYPE "library")
set_target_properties(openvr PROPERTIES FOLDER "3rd")
set_target_properties(openvr_static PROPERTIES FOLDER "_obj")
add_subdirectory(png)
set_target_properties(png PROPERTIES CGVPROP_TYPE "library")
set_target_properties(png PROPERTIES FOLDER "3rd")
set_target_properties(png_static PROPERTIES FOLDER "_obj")
add_subdirectory(screen_capture_lite)
set_target_properties(screen_capture_lite PROPERTIES CGVPROP_TYPE "library")
set_target_properties(screen_capture_lite PROPERTIES FOLDER "3rd")
set_target_properties(screen_capture_lite_static PROPERTIES FOLDER "_obj")
if (NOT MSVC)
        target_compile_options(screen_capture_lite PRIVATE -Wno-unused-variable -Wno-sign-compare)
        target_compile_options(screen_capture_lite_static PRIVATE -Wno-unused-variable -Wno-sign-compare)
endif()

add_subdirectory(tiff)
set_target_properties(tif PROPERTIES
	CGVPROP_TYPE "library"
	FOLDER        "3rd"
	C_STANDARD   "90"
)
set_target_properties(tif_static PROPERTIES
	FOLDER     "_obj"
	C_STANDARD "90"
)

add_subdirectory(xml)
set_target_properties(xml PROPERTIES CGVPROP_TYPE "library")
set_target_properties(xml PROPERTIES FOLDER "3rd")
set_target_properties(xml_static PROPERTIES FOLDER "_obj")

add_subdirectory(zlib)
set_target_properties(zlib PROPERTIES
	CGVPROP_TYPE "library"
	FOLDER       "3rd"
	C_STANDARD   "90"
)
set_target_properties(zlib_static PROPERTIES
	FOLDER     "_obj"
	C_STANDARD "90"
)

if(CGV_BUILD_WITH_AUDIO)
	add_subdirectory(openal-soft)
	add_subdirectory(sndfile)
endif()

if(WIN32)
	# FIXME webserver can only be built on Windows
	add_subdirectory(webserver)
endif()

install(EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
