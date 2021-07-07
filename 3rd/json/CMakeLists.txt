cmake_minimum_required(VERSION 3.15)

project(json)

install(DIRECTORY nlohmann DESTINATION ${CGV_3RD_INCLUDE_DEST} FILES_MATCHING PATTERN "*.hpp")

# Shared Library
add_library(json INTERFACE)
target_include_directories(json INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)

install(TARGETS json EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(json_static INTERFACE)
target_include_directories(json_static INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)

install(TARGETS json_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
