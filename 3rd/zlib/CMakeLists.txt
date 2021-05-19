cmake_minimum_required(VERSION 3.14)

project(zlib)

add_library(zlib
        adler32.c
        compress.c
        crc32.c
        deflate.c
        gzclose.c
        gzlib.c
        gzread.c
        gzwrite.c
        infback.c
        inffast.c
        inflate.c
        inftrees.c
        trees.c
        uncompr.c
        zutil.c)
target_compile_definitions(zlib PRIVATE ZLIB_DLL)

if (MSVC)
    target_compile_options(zlib PRIVATE /w)
else ()
    target_compile_options(zlib PRIVATE -w)
endif ()

target_include_directories(zlib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/zlib>)

install(TARGETS zlib EXPORT cgv_3rd DESTINATION ${CGV_LIB_DEST})
install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/zlib FILES_MATCHING PATTERN "*.h")
