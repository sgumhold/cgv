cmake_minimum_required(VERSION 3.14)

project(tif)

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.c")

add_library(tif ${SOURCES} libtiff.def)
target_link_libraries(tif jpeg)
target_include_directories(tif PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(tif PRIVATE
        _USRDLL
        _CRT_SECURE_NO_DEPRECATE
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
