
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_data ${SOURCES})
target_compile_definitions(cgv_data PRIVATE CGV_DATA_EXPORTS)
target_include_directories(cgv_data PUBLIC
	${CGV_DIR}
)
target_link_libraries(cgv_data PUBLIC cgv_utils cgv_type cgv_reflect)
