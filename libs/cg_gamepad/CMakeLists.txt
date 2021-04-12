
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cg_gamepad ${SOURCES})
target_compile_definitions(cg_gamepad PRIVATE CG_GAMEPAD_EXPORTS)
target_link_libraries(cg_gamepad 
	cgv_utils
	cgv_type
	cgv_reflect
	cgv_data
	cgv_signal
	cgv_os
	cgv_base
	cgv_math
	cgv_media
	cgv_gui
	cgv_reflect_types
	gamepad
)

# FIXME: For unicode support the definitions _UNICODE and UNICODE have to be
# set. Can we do this better?
#cgv_add_library(cg_gamepad 
#	SOURCES ${SOURCES} 
#	PUBLIC_HEADERS ${HEADERS}
#	DEFINITIONS UNICODE _UNICODE
#	SHARED_DEFINITIONS CG_GAMEPAD_EXPORTS)
