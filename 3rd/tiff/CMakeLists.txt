cmake_minimum_required(VERSION 2.6)
project(TIFF)

# Include this file for standard build settings
include("../../cmake/buildCoreComponents.cmake")


# define(defFile="$(CGV_DIR)/3rd/tiff/tiff.def")

set(DEFINES
  "_CRT_SECURE_NO_DEPRECATE"
  "AVOID_WIN32_FILEIO"
  "CHECK_JPEG_YCBCR_SUBSAMPLING"
  "DEFAULT_EXTRASAMPLE_AS_ALPHA"
  "STRIPCHOP_DEFAULT=TIFF_STRIPCHOP"
  "STRIP_SIZE_DEFAULT=8192"
  "LOGLUV_SUPPORT"
  "NEXT_SUPPORT"
  "THUNDER_SUPPORT"
  "LZW_SUPPORT"
  "PACKBITS_SUPPORT"
  "CCITT_SUPPORT"
  "TIF_PLATFORM_CONSOLE"
  "FILLODER_LSB2MSB"
)


cgv_find_package(JPEG)

# Header files
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.c")

include_directories(../jpeg	${JPEG_INCLUDE_DIRS})

cgv_add_3rd_library(TIFF
	SOURCES ${SOURCES} libtiff.def
	PUBLIC_HEADERS ${HEADERS}
	SHARED_DEFINITIONS _USRDLL ${DEFINES}
	STATIC_DEFINITIONS JPEG_STATIC ${DEFINES}
)
	
target_link_libraries(TIFF 
  ${JPEG_LIBRARIES}
)

cgv_write_find_file(TIFF)
