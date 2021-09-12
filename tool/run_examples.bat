REM @ECHO OFF

cgv_viewer^
 plugin:cg_fltk^
 "type(shader_config):shader_path='%CGV_DIR%/cgv/plugins/examples;%CGV_DIR%/cgv/libs/plot/glsl;%CGV_DIR%/cgv/libs/cgv_gl/glsl'"^
 plugin:cg_ext^
 plugin:cg_icons^
 plugin:crg_stereo_view^
 plugin:crg_antialias^
 plugin:crg_depth_of_field^
 plugin:crg_light^
 plugin:cg_meta^
 plugin:cmi_io^
 plugin:cmv_avi^
 plugin:crg_grid^
 plugin:co_web^
 plugin:cmf_tt_gl_font^
 plugin:examples^
 gui:"%CGV_DIR%/cgv/plugins/examples/examples.gui"^
 config:"%CGV_DIR%/cgv/plugins/examples/config.def"
