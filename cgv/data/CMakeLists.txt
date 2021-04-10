
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

add_library(cgv_data ${SOURCES})
target_include_directories(cgv_data PUBLIC
	${CGV_DIR}
)
target_link_libraries(cgv_data PUBLIC cgv_utils cgv_type cgv_reflect)

# Definition of the component
# cgv_add_core_component(data
# 	SOURCES ${SOURCES}
# 	PUBLIC_HEADERS ${PUBLIC_HEADERS}
# 	CGV_DEPENDENCIES utils type reflect)
