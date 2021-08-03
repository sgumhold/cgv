cmake_minimum_required(VERSION 3.15)

project(tif)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c")

set(DEFINES _CRT_SECURE_NO_DEPRECATE
        AVOID_WIN32_FILEIO
        CHECK_JPEG_YCBCR_SUBSAMPLING
        DEFAULT_EXTRASAMPLE_AS_ALPHA
        STRIPCHOP_DEFAULT=TIFF_STRIPCHOP
        STRIP_SIZE_DEFAULT=8192
        LOGLUV_SUPPORT
        NEXT_SUPPORT
        THUNDER_SUPPORT
        LZW_SUPPORT
        PACKBITS_SUPPORT
        CCITT_SUPPORT
        TIF_PLATFORM_CONSOLE
        FILLODER_LSB2MSB)

install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/tif FILES_MATCHING PATTERN "*.h")

# Shared Library
add_library(tif SHARED ${SOURCES} libtiff.def)
target_link_libraries(tif jpeg)
target_compile_definitions(tif PRIVATE
        _USRDLL
        ${DEFINES})

if (MSVC)
    target_compile_options(tif PRIVATE /w)
else ()
    target_compile_options(tif PRIVATE -w)
endif ()

target_include_directories(tif PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/tif>)

install(TARGETS tif EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(tif_static STATIC ${SOURCES} libtiff.def)
target_link_libraries(tif_static jpeg_static)
target_compile_definitions(tif_static PRIVATE ${DEFINES})

if (MSVC)
    target_compile_options(tif_static PRIVATE /w)
else ()
    target_compile_options(tif_static PRIVATE -w)
endif ()

target_include_directories(tif_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/tif>)

install(TARGETS tif_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
