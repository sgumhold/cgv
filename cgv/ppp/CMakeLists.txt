
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

add_library(cgv_ppp ${SOURCES})
target_include_directories(cgv_ppp PUBLIC
	${CGV_DIR}
)
target_link_libraries(cgv_ppp PUBLIC
	cgv_utils
	cgv_type
)
