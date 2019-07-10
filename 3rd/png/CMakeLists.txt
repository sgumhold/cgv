cmake_minimum_required(VERSION 2.6)
project(PNG)

# Include this file for standard build settings
include("../../cmake/buildCoreComponents.cmake")

cgv_find_package(ZLIB)

set(HEADERS
	png.h
	pngconf.h
	pngdebug.h
	pnginfo.h
	pngpriv.h
	pngstruct.h)
	
set(SOURCES
	png.c
	pngerror.c
	pngget.c
	pngmem.c
	pngpread.c
	pngread.c
	pngrio.c
	pngrtran.c
	pngrutil.c
	pngset.c
	pngtrans.c
	pngwio.c
	pngwrite.c
	pngwtran.c
	pngwutil.c)
	
include_directories(
	../zlib
	${ZLIB_INCLUDE_DIRS}
	)

if (WIN32)
	cgv_add_3rd_library(PNG
		SOURCES ${SOURCES}
		PUBLIC_HEADERS ${HEADERS}
		SHARED_DEFINITIONS "PNG_BUILD_DLL" "PNG_NO_MODULEDEF"
		STATIC_DEFINITIONS "PNG_STATIC")
else()
	cgv_add_3rd_library(PNG
		SOURCES ${SOURCES}
		PUBLIC_HEADERS ${HEADERS}
		STATIC_DEFINITIONS "PNG_STATIC")
endif()	

target_link_libraries(PNG ${ZLIB_LIBRARIES})

cgv_write_find_file(PNG)
