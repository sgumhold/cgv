
add_library(openvr_driver openvr_camera.cxx openvr_driver.cxx openvr_kit.cxx)
target_compile_definitions(openvr_driver PRIVATE OPENVR_DRIVER_EXPORTS)
target_link_libraries(openvr_driver glew vr openvr cgv_utils)

install(TARGETS openvr_driver EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
