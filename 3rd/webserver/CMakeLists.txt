cmake_minimum_required(VERSION 3.15)

project(webserver)

add_library(webserver base64.cpp stdHelpers.cpp UrlHelper.cpp webserver.cpp)
target_link_libraries(webserver cgv_os)
target_compile_definitions(webserver PRIVATE WEBSERVER_EXPORT)

if (WIN32)
    target_link_libraries(webserver ws2_32 msimg32)
endif ()

target_include_directories(webserver PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/webserver>)

install(TARGETS webserver EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/webserver FILES_MATCHING PATTERN "*.h")
