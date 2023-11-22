file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

cgv_add_target(cg_audio
	TYPE static_library
	SOURCES ${SOURCES}
	HEADERS ${HEADERS}
	DEPENDENCIES
		cgv_base cgv_gui cgv_oal
)

target_compile_features(cg_audio PUBLIC cxx_std_17)
set_target_properties(cg_audio PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cg_audio PROPERTIES CXX_EXTENSIONS OFF)

install(TARGETS cg_audio EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
