
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

add_library(cgv_gui ${SOURCES})
target_include_directories(cgv_gui PUBLIC
	${CGV_DIR}
)
target_link_libraries(cgv_gui PUBLIC
	cgv_utils
	cgv_type
	cgv_reflect
	cgv_data
	cgv_base
	cgv_media
	cgv_signal
)

# Definition of the component
# cgv_add_core_component(gui
# 	SOURCES ${SOURCES}
# 	PUBLIC_HEADERS ${PUBLIC_HEADERS}
# 	CGV_DEPENDENCIES )

