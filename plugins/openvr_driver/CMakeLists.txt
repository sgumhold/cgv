
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(openvr_driver
	TYPE plugin NO_EXECUTABLE
	SOURCES       ${SOURCES}
	HEADERS       ${HEADERS}
	DEPENDENCIES  glew vr openvr cgv_utils
)

install(TARGETS openvr_driver EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
