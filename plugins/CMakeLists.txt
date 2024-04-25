
if(CGV_BUILD_WITH_AUDIO)
	add_subdirectory(cg_audio)
endif()

# ToDo: old-style targets are temporarily decorated with the CGV-defined type property, remove when
# converting them to the new system
add_subdirectory(camera_animator)
add_subdirectory(cg_ext)
add_subdirectory(cg_icons)
add_subdirectory(cmf_tt_gl_font)
add_subdirectory(cg_fltk)
add_subdirectory(cg_meta)
add_subdirectory(cg_msdf)
add_subdirectory(cmi_io)
add_subdirectory(crg_antialias)
add_subdirectory(crg_depth_of_field)
add_subdirectory(crg_grid)
add_subdirectory(crg_light)
add_subdirectory(crg_bg)
add_subdirectory(crg_stereo_view)
add_subdirectory(crg_holo_view)

add_subdirectory(crg_vr_wall)
add_subdirectory(openvr_driver)
add_subdirectory(crg_vr_view)
add_subdirectory(vr_lab)
add_subdirectory(vr_test)
set_target_properties(vr_test PROPERTIES CGVPROP_TYPE "plugin")

if (WIN32)
    # FIXME these plugins can only be compiled under Windows for now
    add_subdirectory(cmv_avi)
    add_subdirectory(co_web)
endif ()

if (CGV_BUILD_EXAMPLES)
    add_subdirectory(examples)
    set_property(DIRECTORY ${CGV_DIR} PROPERTY VS_STARTUP_PROJECT examples)
endif()

install(EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY . DESTINATION ${CGV_PLUGINS_INCLUDE_DEST} FILES_MATCHING PATTERN "*.h")
