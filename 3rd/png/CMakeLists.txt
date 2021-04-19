cmake_minimum_required(VERSION 3.14)

project(png)

add_library(png
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
target_link_libraries(png zlib)

if (WIN32)
    target_compile_definitions(png PRIVATE PNG_BUILD_DLL PNG_NO_MODULEDEF)
endif ()
