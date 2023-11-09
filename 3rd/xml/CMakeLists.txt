cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

project(xml)

# Shared Library
add_library(xml SHARED ${SOURCES} ${HEADERS})
target_compile_definitions(xml PRIVATE TINYXML2_EXPORT)
target_include_directories(xml PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)

install(TARGETS xml EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(xml_static STATIC ${SOURCES} ${HEADERS})
target_include_directories(xml_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/xml>)

install(TARGETS xml_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
