cmake_minimum_required(VERSION 3.15)

project(png)

set(SOURCES
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

install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/png FILES_MATCHING PATTERN "*.h")

# Shared Library
add_library(png SHARED ${SOURCES})
target_link_libraries(png zlib)

if (WIN32)
    target_compile_definitions(png PRIVATE PNG_BUILD_DLL PNG_NO_MODULEDEF)
endif ()

target_include_directories(png PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/png>)

install(TARGETS png EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(png_static STATIC ${SOURCES})
target_link_libraries(png_static zlib_static)

target_include_directories(png_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/png>)

install(TARGETS png_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
