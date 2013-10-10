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

set(HEADERS
	tiff.h
	tiffio.h
	tiffconf.h
)
	
set(SOURCES
tif_aux.c
tif_close.c
tif_codec.c
tif_color.c
tif_compress.c
tif_dir.c
tif_dirinfo.c
tif_dirread.c
tif_dirwrite.c
tif_dumpmode.c
tif_error.c
tif_extension.c
tif_fax3.c
tif_fax3sm.c
tif_flush.c
tif_getimage.c
tif_jbig.c
tif_jpeg.c
tif_luv.c
tif_lzw.c
tif_next.c
tif_ojpeg.c
tif_open.c
tif_packbits.c
tif_pixarlog.c
tif_predict.c
tif_print.c
tif_read.c
tif_stream.cxx
tif_strip.c
tif_swab.c
tif_thunder.c
tif_tile.c
tif_version.c
tif_warning.c
tif_win32.c
tif_write.c
tif_zip.c
)

include_directories(
	../jpeg
	${JPEG_INCLUDE_DIRS}
	)

cgv_add_3rd_library(TIFF
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${HEADERS}
	SHARED_DEFINITIONS ${DEFINES}
	STATIC_DEFINITIONS ${DEFINES}
)
	
target_link_libraries(TIFF 
  ${JPEG_LIBRARIES}
)

cgv_write_find_file(TIFF)
