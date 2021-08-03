
find_package(OpenGL REQUIRED)

set(SOURCES glsu.c glsu.def)
cgv_create_lib(glsu SOURCES ${SOURCES})

target_link_libraries(glsu ${OPENGL_LIBRARIES})
target_link_libraries(glsu_static ${OPENGL_LIBRARIES})

target_include_directories(glsu PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}/glsu>)

target_include_directories(glsu_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}/glsu>)
