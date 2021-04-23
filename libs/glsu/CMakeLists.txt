
find_package(OpenGL REQUIRED)

add_library(glsu glsu.c glsu.def)
target_link_libraries(glsu ${OPENGL_LIBRARIES})
target_include_directories(glsu PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}/glsu>)

install(TARGETS glsu EXPORT cgv_libs DESTINATION ${CGV_LIB_DEST})
