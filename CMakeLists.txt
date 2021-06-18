cmake_minimum_required(VERSION 3.15)

project(cgv)

set(BUILD_SHARED_LIBS ON)
option(SHADER_DEVELOPER OFF)

set (CMAKE_CXX_STANDARD_REQUIRED 14)

set(CGV_DIR ${PROJECT_SOURCE_DIR})
set(CGV_LIB_DEST lib/cgv)
set(CGV_INCLUDE_DEST include/cgv)
set(CGV_LIBS_INCLUDE_DEST include/cgv-libs)
set(CGV_3RD_INCLUDE_DEST include/cgv-3rd)

# setting up correct RPATH as per: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
set(CMAKE_SKIP_BUILD_RPATH  FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib/cgv" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/cgv")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin PARENT_SCOPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib PARENT_SCOPE)
endif()

add_subdirectory(3rd)
add_subdirectory(tool)
add_subdirectory(cgv)
add_subdirectory(libs)
add_subdirectory(apps)
add_subdirectory(plugins)
add_subdirectory(test)

install(FILES cgv-config.cmake DESTINATION lib)
