cmake_minimum_required(VERSION 3.15)

project(openvr)

set(SOURCES
        src/jsoncpp.cpp
        src/openvr_api_public.cpp
        src/vrcommon/dirtools_public.cpp
        src/vrcommon/envvartools_public.cpp
        src/vrcommon/hmderrors_public.cpp
        src/vrcommon/pathtools_public.cpp
        src/vrcommon/sharedlibtools_public.cpp
        src/vrcommon/strtools_public.cpp
        src/vrcommon/vrpathregistry_public.cpp)

install(DIRECTORY ./headers DESTINATION ${CGV_3RD_INCLUDE_DEST}/openvr FILES_MATCHING PATTERN "*.h")

# Prepare 64bit definitions
if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(OPENVR_PLATFORM_DEFINE
        $<$<PLATFORM_ID:Windows>:WIN64>
        $<$<PLATFORM_ID:Linux>:LINUX>
        $<$<PLATFORM_ID:Linux>:LINUX64>
        $<$<PLATFORM_ID:Linux>:POSIX>)
else()
    set(OPENVR_PLATFORM_DEFINE
        $<$<PLATFORM_ID:Linux>:LINUX>
        $<$<PLATFORM_ID:Linux>:POSIX>)
endif()

# Shared Library
add_library(openvr SHARED ${SOURCES})
target_compile_definitions(openvr PRIVATE
        VR_API_PUBLIC
        OPENVR_API_EXPORTS
        ${OPENVR_PLATFORM_DEFINE})
target_include_directories(openvr PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/headers>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/vrcommon>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/openvr>)

# Static Library
add_library(openvr_static OBJECT ${SOURCES})
target_compile_definitions(openvr_static PRIVATE
        VR_API_PUBLIC
        OPENVR_BUILD_STATIC
        ${OPENVR_PLATFORM_DEFINE})
target_include_directories(openvr_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/headers>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/vrcommon>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/openvr>)

install(TARGETS openvr EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
install(TARGETS openvr_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
