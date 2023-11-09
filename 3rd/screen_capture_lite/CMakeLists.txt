cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#option(BUILD_SHARED_LIBS "Build shared library" ON) 
#option(BUILD_EXAMPLE "Build example" ON)

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
endif()

if(WIN32)
    set(SCREEN_CAPTURE_PLATFORM_SRC     
		src/windows/GetWindows.cpp
		src/windows/GetMonitors.cpp 
		src/windows/DXFrameProcessor.cpp 
		include/windows/DXFrameProcessor.h
		src/windows/GDIFrameProcessor.cpp 
		include/windows/GDIFrameProcessor.h
		src/windows/GDIMouseProcessor.cpp
		include/windows/GDIMouseProcessor.h
		src/windows/ThreadRunner.cpp
		include/windows/GDIHelpers.h
    )
    set(SCREEN_CAPTURE_PLATFORM_INC     
       include/windows 
    )
	add_definitions(-DNOMINMAX)
elseif(APPLE)

    set(SCREEN_CAPTURE_PLATFORM_SRC
        src/ios/GetWindows.cpp 
        src/ios/NSMouseCapture.m
        include/ios/NSMouseCapture.h 
        src/ios/NSFrameProcessor.cpp
        src/ios/NSFrameProcessor.mm
        include/ios/NSFrameProcessorm.h
        include/ios/NSFrameProcessor.h
        src/ios/NSMouseProcessor.cpp
        include/ios/NSMouseProcessor.h
        src/ios/CGFrameProcessor.cpp
        include/ios/CGFrameProcessor.h
        src/ios/GetMonitors.cpp
        src/ios/ThreadRunner.cpp
    )
    set(SCREEN_CAPTURE_PLATFORM_INC
       include/ios 
    )
else()
  set(SCREEN_CAPTURE_PLATFORM_SRC 
       include/linux/X11MouseProcessor.h 
       src/linux/X11MouseProcessor.cpp 
       include/linux/X11FrameProcessor.h 
       src/linux/X11FrameProcessor.cpp
       src/linux/GetMonitors.cpp
       src/linux/GetWindows.cpp
       src/linux/ThreadRunner.cpp
    )
	find_package(X11 REQUIRED)
	if(!X11_XTest_FOUND)
 		message(FATAL_ERROR "X11 extensions are required, but not found!")
	endif()
	if(!X11_Xfixes_LIB)
 		message(FATAL_ERROR "X11 fixes extension is required, but not found!")
	endif()
	set(SCREEN_CAPTURE_PLATFORM_INC
       include/linux 
		${X11_INCLUDE_DIR}
    )
endif()


include_directories(
	include 
	${SCREEN_CAPTURE_PLATFORM_INC} 
)
add_library(screen_capture_lite SHARED
	include/ScreenCapture.h 
	include/internal/SCCommon.h 
	include/internal/ThreadManager.h 
	src/ScreenCapture.cpp 
	src/SCCommon.cpp 
	src/ThreadManager.cpp
	${SCREEN_CAPTURE_PLATFORM_SRC}
)
set_target_properties(screen_capture_lite PROPERTIES DEFINE_SYMBOL SC_LITE_DLL)
if(WIN32) 
    target_link_libraries(screen_capture_lite PRIVATE ${COMMON_LIBRARIES}  Dwmapi)
    if(MSVC)
        install (FILES $<TARGET_PDB_FILE:screen_capture_lite> DESTINATION bin OPTIONAL)
    endif()
elseif(APPLE)
    find_package(Threads REQUIRED)
    find_library(corefoundation_lib CoreFoundation REQUIRED)
    find_library(cocoa_lib Cocoa REQUIRED)
    find_library(coremedia_lib CoreMedia REQUIRED)
    find_library(avfoundation_lib AVFoundation REQUIRED)
    find_library(coregraphics_lib CoreGraphics REQUIRED)
    find_library(corevideo_lib CoreVideo REQUIRED)

    target_link_libraries(
        screen_capture_lite PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
        ${corefoundation_lib}
        ${cocoa_lib}
        ${coremedia_lib}
        ${avfoundation_lib}
        ${coregraphics_lib}
        ${corevideo_lib}
    )
else()
	find_package(X11 REQUIRED)
	if(!X11_XTest_FOUND)
 		message(FATAL_ERROR "X11 extensions are required, but not found!")
	endif()
	if(!X11_Xfixes_LIB)
 		message(FATAL_ERROR "X11 fixes extension is required, but not found!")
	endif()
	find_package(Threads REQUIRED)
	set(screen_capture_lite_PLATFORM_LIBS
		${X11_LIBRARIES}
		${X11_Xfixes_LIB}
		${X11_Xinerama_LIB}
		${CMAKE_THREAD_LIBS_INIT}
	)
	target_link_libraries(screen_capture_lite PRIVATE dl ${COMMON_LIBRARIES} ${screen_capture_lite_PLATFORM_LIBS})
endif()

add_library(screen_capture_lite_static OBJECT
	include/ScreenCapture.h 
	include/internal/SCCommon.h 
	include/internal/ThreadManager.h 
	src/ScreenCapture.cpp 
	src/SCCommon.cpp 
	src/ThreadManager.cpp
	${SCREEN_CAPTURE_PLATFORM_SRC}
)
if(WIN32)
	target_link_libraries(screen_capture_lite_static PUBLIC ${COMMON_LIBRARIES} Dwmapi)
elseif(APPLE)
   target_link_libraries(
        screen_capture_lite_static PUBLIC
        ${CMAKE_THREAD_LIBS_INIT}
        ${corefoundation_lib}
        ${cocoa_lib}
        ${coremedia_lib}
        ${avfoundation_lib}
        ${coregraphics_lib}
        ${corevideo_lib}
    )
else()
	target_link_libraries(screen_capture_lite_static PRIVATE dl ${COMMON_LIBRARIES} ${screen_capture_lite_PLATFORM_LIBS})
endif()

install(TARGETS screen_capture_lite EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
install(TARGETS screen_capture_lite_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
