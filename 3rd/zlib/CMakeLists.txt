cmake_minimum_required(VERSION 2.6)
project(ZLIB)

# Include this file for standard build settings
include("../../cmake/buildCoreComponents.cmake")

set(SOURCES
	adler32.c
	compress.c
	crc32.c
	deflate.c
	gzclose.c
	gzlib.c
	gzread.c
	gzwrite.c
	infback.c
	inffast.c
	inflate.c
	inftrees.c
	trees.c
	uncompr.c
	zutil.c)
	
set(HEADERS
	crc32.h
	deflate.h
	gzguts.h
	inffast.h
	inffixed.h
	inflate.h
	inftrees.h
	trees.h
	zconf.h
	zlib.h
	zutil.h)

cgv_add_3rd_library(ZLIB
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${HEADERS}
	SHARED_DEFINITIONS "ZLIB_DLL")

cgv_write_find_file(ZLIB)
