file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_add_target(cmf_tt_gl_font
	TYPE plugin NO_EXECUTABLE
	SOURCES ${SOURCES}
	DEPENDENCIES cgv_utils cgv_type cgv_base cgv_media cgv_render cgv_gl tt_gl_font glew
)

#target_include_directories(cmf_tt_gl_font PUBLIC
#        $<BUILD_INTERFACE:${CGV_DIR}/libs>
#        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
#install(TARGETS cmf_tt_gl_font EXPORT cgv_libs DESTINATION ${CGV_BIN_DEST})
