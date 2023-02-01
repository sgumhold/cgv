
if(CGV_BUILD_WITH_AUDIO)
	add_subdirectory(cg_audio)
endif()

# ToDo: old-style targets are temporarily decorated with the CGV-defined type property, remove when
# converting them to the new system
add_subdirectory(cg_ext)
set_target_properties(cg_ext PROPERTIES CGVPROP_TYPE "plugin")
add_subdirectory(cg_icons)
add_subdirectory(cmf_tt_gl_font)
add_subdirectory(cg_fltk)
set_target_properties(cg_fltk PROPERTIES CGVPROP_TYPE "plugin")
add_subdirectory(cg_meta)
add_subdirectory(cmi_io)
set_target_properties(cmi_io PROPERTIES CGVPROP_TYPE "plugin")
add_subdirectory(crg_antialias)
add_subdirectory(crg_depth_of_field)
add_subdirectory(crg_grid)
add_subdirectory(crg_light)
add_subdirectory(crg_bg)
add_subdirectory(crg_stereo_view)
set_target_properties(crg_stereo_view PROPERTIES CGVPROP_TYPE "plugin")

add_subdirectory(crg_vr_wall)
set_target_properties(crg_vr_wall PROPERTIES CGVPROP_TYPE "plugin")
add_subdirectory(openvr_driver)
set_target_properties(openvr_driver PROPERTIES CGVPROP_TYPE "plugin")
add_subdirectory(crg_vr_view)
set_target_properties(crg_vr_view PROPERTIES CGVPROP_TYPE "plugin")
add_subdirectory(vr_lab)
set_target_properties(vr_lab PROPERTIES CGVPROP_TYPE "plugin")
add_subdirectory(vr_test)
set_target_properties(vr_test PROPERTIES CGVPROP_TYPE "plugin")

if (WIN32)
    # FIXME these plugins can only be compiled under Windows for now
    add_subdirectory(cmv_avi)
    add_subdirectory(co_web)
endif ()

add_subdirectory(examples)

install(EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(DIRECTORY . DESTINATION ${CGV_PLUGINS_INCLUDE_DEST} FILES_MATCHING PATTERN "*.h")
