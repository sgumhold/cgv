
cgv_prepare_resources(${CMAKE_SOURCE_DIR} IMG_SOURCES cgv_logo.png)

add_executable(cgv_viewer main.cxx ${IMG_SOURCES} resource.rc)
target_link_libraries(cgv_viewer PRIVATE
        cgv_signal
        cgv_base
        cgv_gui
        cgv_render)
set_target_properties(cgv_viewer PROPERTIES CGVPROP_TYPE "app")
install(TARGETS cgv_viewer EXPORT cgv_viewer DESTINATION ${CGV_BIN_DEST})

add_library(cgv_viewer_main OBJECT main.cxx ${IMG_SOURCES})
target_compile_definitions(cgv_viewer_main PRIVATE CGV_FORCE_STATIC)
target_link_libraries(cgv_viewer_main PRIVATE
        cgv_signal_static
        cgv_base_static
        cgv_gui_static
        cgv_render_static)
install(TARGETS cgv_viewer_main EXPORT cgv_viewer DESTINATION ${CGV_BIN_DEST})

install(EXPORT cgv_viewer DESTINATION ${CGV_BIN_DEST})
