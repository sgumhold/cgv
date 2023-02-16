cmake_minimum_required(VERSION 3.20)

# CGV Framework top-level project declaration
project(cgv)

# prelude
include(cgv-options.cmake)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

# make sure we know that we are currently configuring the CGV Framework
set(CGV_IS_CONFIGURING TRUE)

# setup relevant directories
# - "the" CGV directory
set(CGV_DIR ${PROJECT_SOURCE_DIR})
# - install-related directories
set(CGV_BIN_DEST bin)
set(CGV_INCLUDE_DEST include/cgv)
set(CGV_3RD_INCLUDE_DEST include/cgv-3rd)
set(CGV_LIBS_INCLUDE_DEST include/cgv-libs)
set(CGV_PLUGINS_INCLUDE_DEST include/cgv-plugins)
# - correct RPATH handling as per https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/RPATH-handling
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if (NOT APPLE)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
endif ()

# user-settable CGV options
# - option declaration
#   MAINTAINER NOTE: keep in sync with options declared in /define_options.bat
cgv_define_option(
	"SHADER_DEVELOPER" DEFAULT OFF
	DOC "make shader_test throw errors that cause the build process to fail"
)
cgv_define_option(
	"ENCODE_SHADER_BASE64" DEFAULT OFF
	DOC "encode shader codes in base64 before embedding"
)
cgv_define_option(
	"STDCPP17" DEFAULT ON
	DOC "later makes cgv::utils use std::filesystem"
)
cgv_define_option(
	"BUILD_WITH_AUDIO" DEFAULT OFF
	DOC "requires checking out the git submodules when building from the repository"
)
# - handle advanced initialization of CGV options declared above
cgv_init_cgvoptions()
# - CMake-only CGV-related options
option(CGV_BUILD_EXAMPLES "Whether to build the 'examples' plugin which demonstrates various components of the CGV Framework" ON)
option(CGV_BUILD_TESTS "Whether to build the test plugins" OFF)

# customize build directory
# - customize executable/DLL/SO and link library locations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
# - make sure these, the CGV_DIR and the CGV_OPTIONS are propagated to would-be parent CMake build processes
#   that reference the CGV Framework
get_directory_property(hasParent PARENT_DIRECTORY)
if (hasParent)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} PARENT_SCOPE)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} PARENT_SCOPE)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} PARENT_SCOPE)
	set(CGV_DIR ${CGV_DIR} PARENT_SCOPE)
	set(CGV_OPTIONS ${CGV_OPTIONS} PARENT_SCOPE)
endif ()

# setup custom properties
define_property(TARGET
	PROPERTY CGVPROP_TYPE
	BRIEF_DOCS "Indicates target component type"
	FULL_DOCS "Indicates the type of component the target represents (e.g. corelib, plugin etc.)."
)
define_property(TARGET
	PROPERTY CGVPROP_NO_EXECUTABLE
	BRIEF_DOCS "Disables generating a single executable target for plugins"
	FULL_DOCS "For plugins, indicates that no single executable version should be generated."
)
define_property(TARGET
	PROPERTY CGVPROP_SHADERPATH
	BRIEF_DOCS "The shaderpath for this target"
	FULL_DOCS "The list of absolute paths where shaders used by the component are located."
)
define_property(TARGET
	PROPERTY CGVPROP_ADDITIONAL_CMDLINE_ARGS
	BRIEF_DOCS "Custom command line arguments for plugins"
	FULL_DOCS "A list of additional command line arguments for launching the plugin (ignored for non-plugin targets)."
)

# general compiler options
if (NOT MSVC)
	add_compile_options(-Wno-dangling-else -Wno-switch)
endif ()

# testing stuff - TODO: find out what it actually does currently
enable_testing()

# configure each individual part of the framework
add_subdirectory(tool)
add_subdirectory(3rd)
add_subdirectory(cgv)
add_subdirectory(libs)
add_subdirectory(apps)
add_subdirectory(plugins)
add_subdirectory(test)
add_subdirectory(doc)

# install stuff - TODO: found out how the current install system works
install(FILES cgv-config.cmake DESTINATION .)

# we're done configuring the CGV Framework
unset(CGV_IS_CONFIGURING)

# make sure the CGV targets folder structure for IDEs is preserved in including projects
if (NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()
