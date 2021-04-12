
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")
# Files to be preprocessed by PPP
file(GLOB_RECURSE PPP_SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.ph")

ppp_compile("${CGV_DIR}"
	PPP_FILES
	PPP_INCLUDES
	${PPP_SOURCES}
)

add_library(cgv_reflect ${SOURCES} ${PPP_FILES})
target_compile_definitions(cgv_reflect PRIVATE CGV_REFLECT_EXPORTS)
target_include_directories(cgv_reflect PUBLIC
	${CGV_DIR}
	${PPP_INCLUDES}
)
target_link_libraries(cgv_reflect PUBLIC cgv_utils cgv_type)

