cmake_minimum_required(VERSION 2.6)
project(PNG)

# Include this file for standard build settings
include("../../cmake/buildCoreComponents.cmake")

cgv_find_package(ZLIB)

set(HEADERS
	png.h
	pngconf.h)
	
set(SOURCES
	png.c
	pngerror.c
	pnggccrd.c
	pngget.c
	pngmem.c
	pngpread.c
	pngread.c
	pngrio.c
	pngrtran.c
	pngrutil.c
	pngset.c
	pngtrans.c
	pngvcrd.c
	pngwio.c
	pngwrite.c
	pngwtran.c
	pngwutil.c)
	
include_directories(
	../zlib
	${ZLIB_INCLUDE_DIRS}
	)

cgv_add_3rd_library(PNG
	SOURCES ${SOURCES}
	PUBLIC_HEADERS ${HEADERS}
	SHARED_DEFINITIONS "PNG_BUILD_DLL" "PNG_NO_MODULEDEF"
	STATIC_DEFINITIONS "PNG_STATIC")
	
target_link_libraries(PNG ${ZLIB_LIBRARIES})

cgv_write_find_file(PNG)
