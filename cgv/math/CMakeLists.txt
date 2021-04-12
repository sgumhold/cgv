
# Source files
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
# Header files
file(GLOB_RECURSE PUBLIC_HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.h")

add_library(cgv_math ${SOURCES})
target_compile_definitions(cgv_math PRIVATE CGV_MATH_EXPORTS)
target_include_directories(cgv_math PUBLIC
	${CGV_DIR}
)
