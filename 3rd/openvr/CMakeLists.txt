cmake_minimum_required(VERSION 3.15)

project(openvr)

add_library(openvr
        src/jsoncpp.cpp
        src/openvr_api_public.cpp
        src/vrcommon/dirtools_public.cpp
        src/vrcommon/envvartools_public.cpp
        src/vrcommon/hmderrors_public.cpp
        src/vrcommon/pathtools_public.cpp
        src/vrcommon/sharedlibtools_public.cpp
        src/vrcommon/strtools_public.cpp
        src/vrcommon/vrpathregistry_public.cpp)
target_compile_definitions(openvr PRIVATE
        VR_API_PUBLIC
        $<$<PLATFORM_ID:Linux>:LINUX>
        $<$<PLATFORM_ID:Linux>:POSIX>)
target_include_directories(openvr PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/headers>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/vrcommon>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/openvr>)

install(TARGETS openvr EXPORT cgv_3rd DESTINATION ${CGV_LIB_DEST})
install(DIRECTORY ./headers DESTINATION ${CGV_3RD_INCLUDE_DEST}/openvr FILES_MATCHING PATTERN "*.h")
