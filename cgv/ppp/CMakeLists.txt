
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_ppp ${SOURCES})
target_compile_definitions(cgv_ppp PRIVATE CGV_PPP_EXPORTS)
target_include_directories(cgv_ppp PUBLIC ${CGV_DIR})
target_link_libraries(cgv_ppp PUBLIC
	cgv_utils
	cgv_type
)
