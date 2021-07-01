cmake_minimum_required(VERSION 3.15)

project(jpeg)

add_library(jpeg
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
install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/jpeg FILES_MATCHING PATTERN "*.h")
