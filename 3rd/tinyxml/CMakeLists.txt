cmake_minimum_required(VERSION 3.15)

project(tinyxml)

install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/tinyxml FILES_MATCHING PATTERN "*.h")

# Shared Library
add_library(tinyxml INTERFACE)
target_compile_definitions(tinyxml PRIVATE TINYXML2_EXPORT)
target_include_directories((tinyxml  INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)

install(TARGETS . EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(json_static INTERFACE)
target_include_directories(tinyxml_static INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/tinyxml>)

install(TARGETS tinyxml_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
