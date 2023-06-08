cmake_minimum_required(VERSION 3.15)

file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

project(tinyxml)

#install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/tinyxml FILES_MATCHING PATTERN "*.h")

# Shared Library
add_library(tinyxml SHARED ${SOURCES} ${HEADERS})
target_compile_definitions(tinyxml PRIVATE TINYXML2_EXPORT)
target_include_directories(tinyxml PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)

install(TARGETS tinyxml EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(tinyxml_static ${SOURCES} ${HEADERS})
target_include_directories(tinyxml_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/tinyxml>)

install(TARGETS tinyxml_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
