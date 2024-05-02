cmake_minimum_required(VERSION 3.24)

# CGV Framework top-level project declaration
project(cgv)

# prelude
include(CheckIPOSupported)
include(cgv-options.cmake)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_CXX_STANDARD_REQUIRED 17)
check_ipo_supported(RESULT IS_IPO_SUPPORTED)

# make sure we know that we are currently configuring the CGV Framework
set(CGV_IS_CONFIGURING TRUE)

# detect whether we're being included as a subproject
get_directory_property(CGV_HAS_PARENT_PROJ PARENT_DIRECTORY)

# detect whether we're configuring for a multi-config generator
get_property(CGV_USING_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(CGV_HAS_PARENT_PROJ)
	set(CGV_USING_MULTI_CONFIG ${CGV_USING_MULTI_CONFIG} PARENT_SCOPE)
endif()

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
	"NO_OPENVR" DEFAULT OFF
	DOC "allows to avoid the start of openvr during development"
)
cgv_define_option(
	"SHADER_DEVELOPER" DEFAULT OFF
	DOC "make shader_test throw errors that cause the build process to fail"
)
cgv_define_option(
	"ENCODE_SHADER_BASE64" DEFAULT OFF
	DOC "encode shader codes in base64 before embedding"
)
cgv_define_option(
	"STDCPP17" DEFAULT OFF
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
if (IS_IPO_SUPPORTED)
	option(CGV_LTO_ON_RELEASE "Whether to compile release builds with whole-program / link-time optimization enabled" ON)
else()
	# make sure we can't set this option when unsupported
	set(CGV_LTO_ON_RELEASE OFF CACHE INTERNAL "" FORCE)
endif()
if (NOT CMAKE_SOURCE_DIR STREQUAL CGV_DIR)
	option(CGV_EXCLUDE_UNUSED_TARGETS
		"When the CGV Framework is used as a dependency from an external project, this option will exclude all CGV targets not needed by the external project form the \"build all\" operation"
		ON
	)
else()
	# this option makes no sense when stand-alone building the CGV Framework directly
	set(CGV_EXCLUDE_UNUSED_TARGETS OFF CACHE INTERNAL "" FORCE)
endif()
if (UNIX)
	option(CGV_DEBUG_ON_NVIDIA_GPU "Adds environment variables to IDE debug configs that should cause the viewer application to run on an NVIDIA GPU if present." OFF)
else()
	set(CGV_DEBUG_ON_NVIDIA_GPU OFF CACHE INTERNAL "" FORCE)
endif()

# customize build directory
# - customize executable/DLL/SO and link library locations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
# - make sure these, the CGV_DIR and the CGV_OPTIONS are propagated to would-be parent CMake build processes that reference the CGV Framework
if (CGV_HAS_PARENT_PROJ)
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
define_property(TARGET
	PROPERTY CGVPROP_WORKING_DIR
	BRIEF_DOCS "Custom working directory used in plugin launch/debug configurations"
	FULL_DOCS "The working directory to use when creating launch/debug configurations for the target (ignored for non-plugin targets)."
)
define_property(TARGET
	PROPERTY CGVPROP_INVOCATION_PROXY
	BRIEF_DOCS "Custom proxy program or script that should launch the viewer application (or single-exe plugin build)."
	FULL_DOCS "Custom proxy program or script that should be invoked instead of launching the viewer application (or single-exe plugin build) directly."
)
define_property(GLOBAL
	PROPERTY CGVPROP_USER_TARGETS
	BRIEF_DOCS "List of CGV component targets added by a user of the CGV Framework"
	FULL_DOCS "List of all CGV components known to the build system that were not added by the Framework itself, but an external project."
)

# general compiler options
if (NOT MSVC)
	add_compile_options(-Wno-dangling-else -Wno-switch)
	if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR MAKE_CXX_COMPILER_ID MATCHES "Clang")
		set(CGV_CLANG_SPECIFIC_DEBUG_FLAGS $<$<CONFIG:Debug,RelWithDebInfo>:-fstandalone-debug -fno-limit-debug-info>)
		if(CGV_HAS_PARENT_PROJ)
			set(CGV_CLANG_SPECIFIC_DEBUG_FLAGS ${CGV_CLANG_SPECIFIC_DEBUG_FLAGS} PARENT_SCOPE)
		endif()
	endif()
endif()

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
