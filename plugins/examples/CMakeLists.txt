
# compile a list of source files for each specific source type the CGV CMake build system knows about
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png" "*.bmp")
file(GLOB_RECURSE AUDIO RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.wav")
if(NOT CGV_BUILD_WITH_AUDIO)
	list(REMOVE_ITEM SOURCES "spatial_audio.cxx")
endif()

# add our target to the CGV CMake build system
cgv_add_target(examples
	TYPE plugin
	SOURCES         ${SOURCES}
	PPP_SOURCES     ${PPP_SOURCES}
	HEADERS         ${HEADERS}
	SHADER_SOURCES  ${SHADERS}
	RESOURCES       ${IMAGES}
	AUDIO_RESOURCES ${AUDIO}	# <-- inclusion of audio resources is automatically disabled when BUILD_WITH_AUDIO is off
	DEPENDENCIES
		delaunay plot cg_gamepad cgv_proc cgv_gl cgv_os cgv_signal cgv_g2d cgv_gpgpu cgv_app
		cgv_viewer cg_ext cg_fltk cg_icons cg_meta cmi_io cgv_proc cmf_tt_gl_font crg_antialias
		crg_depth_of_field crg_grid crg_light crg_bg crg_stereo_view
	ADDITIONAL_CMDLINE_ARGS
		"gui:\"${CMAKE_CURRENT_LIST_DIR}/examples.gui\"" "config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""

	# we are not overriding the default preprocesser define indicating that the shared library is being
	# build and symbols should be exported, but if we wanted to, we could
	# OVERRIDE_SHARED_EXPORT_DEFINE I_AM_EXPORTING

	# we are not overriding the default preprocesser define indicating that we are building the static
	# version for use by single executables, but if we wanted to, we could
	# OVERRIDE_FORCE_STATIC_DEFINE  I_AM_STATIC
)

# TODO: provide some obvious and canonical way of accomplishing conditional inclusion of dependencies for CGV targets
if(CGV_BUILD_WITH_AUDIO)
	target_link_libraries(examples PRIVATE cg_audio cgv_oal)
	target_link_libraries(examples_static PUBLIC cg_audio_static cgv_oal_static)
	target_link_libraries(examples_exe PRIVATE cg_audio_static cgv_oal_static)
endif()
if(WIN32)
	target_link_libraries(examples PRIVATE cmv_avi co_web)
	target_link_libraries(examples_static PUBLIC cmv_avi_static co_web_static)
	target_link_libraries(examples_exe PRIVATE cmv_avi_static co_web_static)
endif()

# FIXME: CMake installs are currently still unsupported and need updating for the new CMake build system
install(TARGETS examples EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(FILES config.def examples.gui DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/examples)
install(DIRECTORY . DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/examples FILES_MATCHING PATTERN "*.gl*")
