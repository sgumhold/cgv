#!/usr/bin/env bash

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
LIBS_DIR=${SCRIPT_DIR}/../include/cgv-libs
PLUGINS_DIR=${SCRIPT_DIR}/../include/cgv-plugins

${SCRIPT_DIR}/cgv_viewer \
  plugin:cg_fltk \
  "type(shader_config):shader_path='${PLUGINS_DIR}/examples;${LIBS_DIR}/plot/glsl;${LIBS_DIR}/cgv_gl/glsl'" \
  plugin:cg_ext \
  plugin:cg_icons \
  plugin:crg_stereo_view \
  plugin:crg_antialias \
  plugin:crg_depth_of_field \
  plugin:crg_light \
  plugin:cg_meta \
  plugin:cmi_io \
  plugin:cmv_avi \
  plugin:crg_grid \
  plugin:co_web \
  plugin:cmf_tt_gl_font \
  plugin:examples \
  gui:"${PLUGINS_DIR}/examples/examples.gui" \
  config:"${PLUGINS_DIR}/examples/config.def"
