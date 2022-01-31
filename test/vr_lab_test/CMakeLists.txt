file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(vr_lab_test ${SOURCES})
target_link_libraries(vr_lab_test vr_lab plot)
add_dependencies(vr_lab_test cg_fltk cmf_tt_gl_font crg_grid cg_ext cmi_io crg_vr_view vr_lab vr_emulator openvr_driver)

set_plugin_execution_params(vr_lab_test "plugin:cg_fltk \
\"type(shader_config):shader_path='${CGV_DIR}/test/vr_lab_test;${CGV_DIR}/plugins/crg_vr_view;${CGV_DIR}/plugins/crg_vr_lab;${CGV_DIR}/libs/cgv_gl/glsl;${CGV_DIR}/libs/cgv_proc/glsl;${CGV_DIR}/libs/plot/glsl'\" \
plugin:cmf_tt_gl_font \
plugin:crg_grid \
plugin:cg_ext \
plugin:cg_icons \
plugin:cmi_io \
plugin:crg_vr_view \
plugin:vr_lab \
plugin:vr_emulator \
plugin:openvr_driver \
plugin:vr_lab_test \
config:\"${CGV_DIR}/test/vr_lab_test/config.def\"\
")

install(TARGETS vr_lab_test EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
install(FILES config.def DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/vr_lab_test)
install(DIRECTORY . DESTINATION ${CGV_PLUGINS_INCLUDE_DEST}/vr_lab_test FILES_MATCHING PATTERN "*.gl*")
