
find_package(OpenGL REQUIRED)

cgv_create_lib(glsu SOURCES glsu.c)

if (WIN32)
    # make sure the .def file doesn't end up in the glsu_static target (which is an
    # object library which must not contain .def files as per CMake regulations)
    target_sources(glsu PRIVATE glsu.def)
endif()

target_link_libraries(glsu ${OPENGL_LIBRARIES})
target_link_libraries(glsu_static ${OPENGL_LIBRARIES})

target_include_directories(glsu PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}/glsu>)

target_include_directories(glsu_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}/glsu>)
