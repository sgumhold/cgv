cmake_minimum_required(VERSION 3.15)

project(glew)

find_package(OpenGL REQUIRED)

add_library(glew glew.c)
target_link_libraries(glew ${OPENGL_LIBRARIES})
target_compile_definitions(glew PRIVATE GLEW_BUILD)
target_include_directories(glew PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}>)
if(APPLE)
   target_include_directories(glew PUBLIC "/opt/X11/include")
endif()

install(TARGETS glew EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY GL DESTINATION ${CGV_3RD_INCLUDE_DEST} FILES_MATCHING PATTERN "*.h")
