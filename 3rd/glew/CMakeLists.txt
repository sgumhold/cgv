cmake_minimum_required(VERSION 3.15)

project(glew)

find_package(OpenGL REQUIRED)

install(DIRECTORY GL DESTINATION ${CGV_3RD_INCLUDE_DEST} FILES_MATCHING PATTERN "*.h")

# Shared Library
add_library(glew SHARED glew.c)
target_link_libraries(glew ${OPENGL_LIBRARIES})
target_compile_definitions(glew PRIVATE GLEW_BUILD)
target_include_directories(glew PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)
if(APPLE)
   target_include_directories(glew PUBLIC "/opt/X11/include")
endif()

install(TARGETS glew EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(glew_static STATIC glew.c)
target_link_libraries(glew_static ${OPENGL_LIBRARIES})
target_compile_definitions(glew_static PUBLIC GLEW_STATIC)
target_include_directories(glew_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)
if(APPLE)
   target_include_directories(glew_static PUBLIC "/opt/X11/include")
endif()

install(TARGETS glew_static EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
