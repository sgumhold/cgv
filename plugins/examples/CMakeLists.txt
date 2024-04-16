
# compile a list of source files for each specific source type the CGV CMake build system knows about
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx" "*.cpp")
file(GLOB_RECURSE HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.gl*")
file(GLOB_RECURSE IMAGES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.png" "*.bmp")
file(GLOB_RECURSE AUDIO RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.wav")

# TODO: provide some obvious and canonical way of accomplishing conditional inclusion of sources and dependencies for CGV targets
if(NOT CGV_BUILD_WITH_AUDIO)
	list(REMOVE_ITEM SOURCES "spatial_audio.cxx")
else()
	set(AUDIO_DEPENDENCIES cg_audio cgv_oal)
endif()
if(WIN32)
	set(WINDOWS_DEPENDENCIES cmv_avi co_web)
endif()

# add our target to the CGV CMake build system
cgv_add_target(examples
	TYPE plugin     # if we didn't want a single executable target to be generated, we could add the NO_EXECUTABLE flag
	SOURCES         ${SOURCES}
	PPP_SOURCES     ${PPP_SOURCES}
	HEADERS         ${HEADERS}
	SHADER_SOURCES  ${SHADERS}
	RESOURCES       ${IMAGES}
	AUDIO_RESOURCES ${AUDIO}	# <-- inclusion of audio resources is automatically disabled when BUILD_WITH_AUDIO is off
	DEPENDENCIES
		cgv_viewer delaunay plot cg_gamepad holo_disp cgv_proc cgv_gl cgv_os cgv_signal cgv_g2d cgv_gpgpu cgv_app cgv_post
		cg_ext cg_fltk cg_icons cg_meta cmi_io cgv_proc cmf_tt_gl_font crg_antialias crg_depth_of_field
		crg_grid crg_light crg_holo_view camera_animator ${AUDIO_DEPENDENCIES} ${WINDOWS_DEPENDENCIES}

	# we are not using symbols from other plugins directly anywhere in the examples code, but if we did, we could specify them
	# here so the linker would know to look for these symbols in the import libs / shared objects of the listed plugins
	# LINKTIME_PLUGIN_DEPENDENCIES crg_bg

	ADDITIONAL_CMDLINE_ARGS
		"gui:\"${CMAKE_CURRENT_LIST_DIR}/examples.gui\"" "config:\"${CMAKE_CURRENT_LIST_DIR}/config.def\""

	# we are not overriding the default preprocesser define that indicates we are building the shared library and symbols
	# should be exported (in this case, the default is EXAMPLES_EXPORTS), but if we wanted to, we could
	# OVERRIDE_SHARED_EXPORT_DEFINE I_AM_EXPORTING

	# we are not overriding the default preprocesser define that indicates we are building the static version for use by
	# single executables (in this case, the default is EXAMPLES_FORCE_STATIC), but if we wanted to, we could
	# OVERRIDE_FORCE_STATIC_DEFINE  I_AM_STATIC

	# we don't need any additional defines private or public to the build of the 'examples' plugin, but if we did,
	# we could declare them here
	# ADDITIONAL_PRIVATE_DEFINES  SOME_INTERNAL_PREPROCESSOR_MACRO
	# ADDITIONAL_PUBLIC_DEFINES   SOME_PUBLICALLY_VISIBLE_PREPROCESSER_MACRO
)

# Visual Studio fluff
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT examples)

# FIXME: CMake installs are currently still unsupported and need updating for the new CMake build system
install(TARGETS examples EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(FILES config.def examples.gui DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/examples)
install(DIRECTORY . DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/examples FILES_MATCHING PATTERN "*.gl*")
