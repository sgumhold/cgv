cmake_minimum_required(VERSION 2.6)
project(tests)

if(BUILD_VR_SUPPORT)
	# Build the VR emulator
	add_subdirectory(vr)
	# The VR Emulator depend on the CGV framework and 3rd party libs
	add_dependencies(vr_emulator cgv 3rd)
endif()

add_custom_target(tests)

if(BUILD_VR_SUPPORT)
	add_dependencies(tests cgv_gl glsu plot cgv_reflect_types delaunay gamepad cg_gamepad vr cg_vr vr_emulator)
else()
	add_dependencies(tests cgv_gl glsu plot cgv_reflect_types delaunay gamepad cg_gamepad)
endif()

set_target_properties(tests PROPERTIES FOLDER "${FOLDER_NAME_LIBS}")
