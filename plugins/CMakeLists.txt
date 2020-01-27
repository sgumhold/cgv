cmake_minimum_required(VERSION 2.6)
project(plugins)

add_subdirectory(cg_fltk)
add_subdirectory(crg_grid)
add_subdirectory(crg_stereo_view)
add_subdirectory(crg_vr_view)
add_subdirectory(crg_vr_wall)
add_subdirectory(cmi_io)
add_subdirectory(cg_icons)
add_subdirectory(cg_ext)
add_subdirectory(crg_light)

if(BUILD_EXAMPLE_PLUGIN)
	add_subdirectory(examples)
endif()

add_custom_target(plugins)
if(BUILD_EXAMPLE_PLUGIN)
	add_dependencies(plugins cg_fltk crg_grid crg_stereo_view cmi_io cg_icons cg_ext crg_light examples)
else()
	add_dependencies(plugins cg_fltk crg_grid crg_stereo_view cmi_io cg_icons cg_ext crg_light)
endif()

set_target_properties(plugins PROPERTIES FOLDER "${FOLDER_NAME_TOPLEVEL}")



