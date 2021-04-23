
cgv_prepare_resources(${CMAKE_SOURCE_DIR} IMG_SOURCES cgv_logo.png)

add_executable(cgv_viewer main.cxx ${IMG_SOURCES})
target_link_libraries(cgv_viewer
        cgv_signal
        cgv_base
        cgv_gui
        cgv_render)

install(TARGETS cgv_viewer EXPORT cgv_viewer DESTINATION ${CGV_LIB_DEST})
install(EXPORT cgv_viewer DESTINATION ${CGV_LIB_DEST})
