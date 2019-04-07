# In a normal local environment this file is expected to be in one sub folder
# of the cgv distribution (be it the installation or source tree), so the 
# CGV_BASE variable is set relative to the position of this list file
get_filename_component(BASE_PATH "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(CGV_BASE "${BASE_PATH}" PATH)

# Create a name depending on the operating system
# FIXME: This name shall better be generated depending on the compiler or
#        even better depending on the capabilities of the compiler
set(OS_PREFIX "unknown")

if (UNIX)
	set(OS_PREFIX "unix")
elseif (WIN32)
	set(OS_PREFIX "win32")
elseif (APPLE)
	set(OS_PREFIX "apple")
endif()

# For installation set installation prexices
set(INSTALL_BASE "framework")
set(INSTALL_HEADER_PATH "include")
set(INSTALL_SHADER_PATH "shader")
set(INSTALL_RESOURCE_PATH "res")
set(INSTALL_LIB_PATH "lib/${OS_PREFIX}")
set(INSTALL_BIN_PATH "bin/${OS_PREFIX}")
set(INSTALL_CMAKE_PATH "cmake/${OS_PREFIX}")

# Set the build path name for out-of-tree compilations of the framework
set(BUILD_BASE "build")

# Set the folder names for VisualStudios virtual project folders
set(FOLDER_NAME_CGV "CGV-Framework")
set(FOLDER_NAME_TOOL "Core Tools")
set(FOLDER_NAME_APPS "Applications")
set(FOLDER_NAME_LIBS "Additional Libraries")
set(FOLDER_NAME_3RD "3rd-Party Libraries")
set(FOLDER_NAME_PLUGINS "Standard Plugins")
set(FOLDER_NAME_APPLICATION_PLUGINS "Application Plugins")
set(FOLDER_NAME_TOPLEVEL "Toplevel Targets")

# Set the name of the cgv-export files
set(CGV_EXPORTS_ID "cgv_exports")
set(CGV_EXPORTS_NAME "${CGV_EXPORTS_ID}.cmake")
set(CGV_EXPORTS_INDEX_NAME "${CGV_EXPORTS_ID}_index.cmake")
set(CGV_TARGETS_CONFIG_NAME "cgvTargetsConfig.cmake")

set (PLATFORM_POSTFIX "")
set (GENERATOR_POSTFIX "")
if(${CMAKE_GENERATOR} STREQUAL "Visual Studio 15 2017")
	set (GENERATOR_POSTFIX "141")
elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 15 2017 Win64")
	set (GENERATOR_POSTFIX "141")
	set (PLATFORM_POSTFIX "64")
elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015")
	set (GENERATOR_POSTFIX "14")
elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 14 2015 Win64")
	set (GENERATOR_POSTFIX "14")
	set (PLATFORM_POSTFIX "64")
elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 12 2013")
	set (GENERATOR_POSTFIX "12")
elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 12 2013 Win64")
	set (GENERATOR_POSTFIX "12")
	set (PLATFORM_POSTFIX "64")
elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11 2012")
	set (GENERATOR_POSTFIX "11")
elseif(${CMAKE_GENERATOR} STREQUAL "Visual Studio 11 2012 Win64")
	set (GENERATOR_POSTFIX "11")
	set (PLATFORM_POSTFIX "64")
endif()

# Set postfixes for results
set(CGV_POSTFIX ${GENERATOR_POSTFIX})
if (CGV_POSTFIX)
	set(CGV_POSTFIX "_${CGV_POSTFIX}")
endif()
set(CGV_POSTFIX "${PLATFORM_POSTFIX}${CGV_POSTFIX}")
set(CGV_DEBUG_POSTFIX "${PLATFORM_POSTFIX}_d${GENERATOR_POSTFIX}")
set(CGV_STATIC_POSTFIX "${PLATFORM_POSTFIX}_s${GENERATOR_POSTFIX}")
set(CGV_STATIC_DEBUG_POSTFIX "${PLATFORM_POSTFIX}_sd${GENERATOR_POSTFIX}")

# Set CXX standard to c++11
set(CXX_STANDARD 11)

if (UNIX)
	# Disable narrowing errors, because this is violated everywhere
	add_definitions(-Wno-narrowing)
elseif (WIN32)
	add_definitions(/EHsc /D "WIN32")
endif ()


# Enable source folders
# FIXME: Make this an option for VS Express?
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)


