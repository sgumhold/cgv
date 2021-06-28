cmake_minimum_required(VERSION 3.15)

project(png)

add_library(png
        png.c
        pngerror.c
        pngget.c
        pngmem.c
        pngpread.c
        pngread.c
        pngrio.c
        pngrtran.c
        pngrutil.c
        pngset.c
        pngtrans.c
        pngwio.c
        pngwrite.c
        pngwtran.c
        pngwutil.c)
target_link_libraries(png zlib)

if (WIN32)
    target_compile_definitions(png PRIVATE PNG_BUILD_DLL PNG_NO_MODULEDEF)
endif ()

target_include_directories(png PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/png>)

install(TARGETS png EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/png FILES_MATCHING PATTERN "*.h")
