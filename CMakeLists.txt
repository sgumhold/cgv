cmake_minimum_required(VERSION 3.15)

project(cgv)

option(CGV_SHADER_DEVELOPER "make shader_test throw errors that make the build process fail" OFF)
option(CGV_BUILD_WITH_AUDIO "requires checking out the git submodules when building from the repository" OFF)
option(CGV_BUILD_TESTS OFF)


set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

set(CGV_DIR ${PROJECT_SOURCE_DIR})
set(CGV_BIN_DEST bin)
set(CGV_INCLUDE_DEST include/cgv)
set(CGV_3RD_INCLUDE_DEST include/cgv-3rd)
set(CGV_LIBS_INCLUDE_DEST include/cgv-libs)
set(CGV_PLUGINS_INCLUDE_DEST include/cgv-plugins)

# setting up correct RPATH as per: https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if (NOT APPLE)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
get_directory_property(hasParent PARENT_DIRECTORY)
if (hasParent)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin PARENT_SCOPE)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib PARENT_SCOPE)
endif ()

if (NOT MSVC)
    add_compile_options(-Wno-dangling-else -Wno-switch)
endif ()

enable_testing()

add_subdirectory(3rd)
add_subdirectory(tool)
add_subdirectory(cgv)
add_subdirectory(libs)
add_subdirectory(apps)
add_subdirectory(plugins)
add_subdirectory(test)
add_subdirectory(doc)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT examples)

install(FILES cgv-config.cmake DESTINATION .)
