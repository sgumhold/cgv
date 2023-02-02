cmake_minimum_required(VERSION 3.15)

project(webserver)

file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(webserver
	TYPE library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES ws2_32 msimg32 cgv_os
	OVERRIDE_SHARED_EXPORT_DEFINE WEBSERVER_EXPORT
)

# TODO: investigate what this does, if it is necessary and if yes, how to best handle it automatically within the CGV CMake infrastructure
target_include_directories(webserver PUBLIC $<INSTALL_INTERFACE:${CGV_3RD_INCLUDE_DEST}/webserver>)

#install(TARGETS webserver EXPORT cgv_3rd DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY . DESTINATION ${CGV_3RD_INCLUDE_DEST}/webserver FILES_MATCHING PATTERN "*.h")
