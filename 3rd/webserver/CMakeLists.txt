cmake_minimum_required(VERSION 3.14)

project(webserver)

add_library(webserver base64.cpp stdHelpers.cpp UrlHelper.cpp webserver.cpp)
target_link_libraries(webserver cgv_os)
target_include_directories(webserver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(webserver PRIVATE WEBSERVER_EXPORT)

if (WIN32)
    target_link_libraries(webserver ws2_32 msimg32)
endif ()
