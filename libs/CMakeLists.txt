
add_subdirectory(cg_gamepad)
add_subdirectory(cgv_gl)
add_subdirectory(cgv_reflect_types)
add_subdirectory(delaunay)
add_subdirectory(gamepad)
add_subdirectory(glsu)
add_subdirectory(plot)

install(FILES cgv-libs-config.cmake DESTINATION ${CGV_LIBS_LIB_DEST})
install(DIRECTORY . DESTINATION ${CGV_LIBS_INCLUDE_DEST} FILES_MATCHING PATTERN "*.h")
# TODO find out why there are *.hh files (maybe rename them to *.h)
install(DIRECTORY . DESTINATION ${CGV_LIBS_INCLUDE_DEST} FILES_MATCHING PATTERN "*.hh")
