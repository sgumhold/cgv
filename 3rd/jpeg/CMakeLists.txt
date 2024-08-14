cmake_minimum_required(VERSION 3.15)

project(jpeg)

set(SOURCES
        jcapimin.c
        jcapistd.c
        jccoefct.c
        jccolor.c
        jcdctmgr.c
        jchuff.c
        jcinit.c
        jcmainct.c
        jcmarker.c
        jcmaster.c
        jcomapi.c
        jcparam.c
        jcphuff.c
        jcprepct.c
        jcsample.c
        jctrans.c
        jdapimin.c
        jdapistd.c
        jdatadst.c
        jdatasrc.c
        jdcoefct.c
        jdcolor.c
        jddctmgr.c
        jdhuff.c
        jdinput.c
        jdmainct.c
        jdmarker.c
        jdmaster.c
        jdmerge.c
        jdphuff.c
        jdpostct.c
        jdsample.c
        jdtrans.c
        jerror.c
        jfdctflt.c
        jfdctfst.c
        jfdctint.c
        jidctflt.c
        jidctfst.c
        jidctint.c
        jidctred.c
        jmemansi.c
        jmemmgr.c
        jquant1.c
        jquant2.c
        jutils.c)

install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/jpeg FILES_MATCHING PATTERN "*.h")

# Shared Library
add_library(jpeg SHARED ${SOURCES})
target_compile_definitions(jpeg PRIVATE JPEG_DLL)

if (MSVC)
    target_compile_options(jpeg PRIVATE /w)
else ()
    target_compile_options(jpeg PRIVATE -w)
endif ()

target_include_directories(jpeg PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/jpeg>)

install(TARGETS jpeg EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(jpeg_static STATIC ${SOURCES})
target_compile_definitions(jpeg_static PUBLIC All_STATIC JPEG_STATIC)

if (MSVC)
    target_compile_options(jpeg_static PRIVATE /w)
else ()
    target_compile_options(jpeg_static PRIVATE -w)
endif ()

if (NOT WIN32)
    target_compile_options(jpeg_static PUBLIC -fPIC)
endif()

target_include_directories(jpeg_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/jpeg>)

install(TARGETS jpeg_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
