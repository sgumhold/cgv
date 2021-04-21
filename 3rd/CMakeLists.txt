
add_subdirectory(fltk)
add_subdirectory(glew)
add_subdirectory(jpeg)
add_subdirectory(json)
add_subdirectory(png)
add_subdirectory(tiff)
add_subdirectory(zlib)

if (WIN32)
    # FIXME webserver can only be built on Windows
    add_subdirectory(webserver)
endif ()
