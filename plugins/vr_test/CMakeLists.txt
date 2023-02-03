
# TODO this fixes this error:
#   The imported target "vr_test" references the file ".../cgv-install/lib/cgv/vr_test.lib" but this file does not exist.
#   it is unclear whether this causes any other unintended side effects
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(vr_test vr_test.cxx)
target_link_libraries(vr_test
        cgv_utils cgv_type cgv_data cgv_base cgv_math
        cgv_media cgv_gui cgv_render glew crg_vr_view)
target_include_directories(vr_test PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<BUILD_INTERFACE:${CGV_DIR}/plugins/crg_vr_view>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)
add_dependencies(vr_test
        cgv_viewer

        cmi_io
        cg_fltk crg_grid cg_ext cgv_gl
        crg_vr_wall
        cg_vr vr_emulator openvr_driver)

set_plugin_execution_params(vr_test ARGUMENTS "plugin:cmi_io \
plugin:cg_fltk \
\"type(shader_config):shader_path='${CGV_DIR}/plugins/vr_test;${CGV_DIR}/plugins/crg_vr_view;${CGV_DIR}/libs/cgv_gl/glsl'\" \
plugin:crg_grid \
plugin:cg_ext \
plugin:cg_icons \
plugin:crg_vr_view \
plugin:crg_vr_wall \
plugin:vr_emulator \
plugin:openvr_driver \
plugin:vr_test \
config:\"${CGV_DIR}/plugins/vr_test/config.def \
")

install(TARGETS vr_test EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
