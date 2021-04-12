
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

add_library(cgv_reflect_types ${SOURCES})
target_include_directories(cgv_reflect_types PUBLIC
	${CGV_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../
)
target_link_libraries(cgv_reflect_types
	cgv_utils
	cgv_type
	cgv_reflect
	cgv_media
	cgv_math
	cgv_render
	cgv_gui
)

# DEFINITIONS UNICODE _UNICODE
# SHARED_DEFINITIONS CGV_REFLECT_TYPES_EXPORTS)
