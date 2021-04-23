
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_math ${SOURCES})
target_compile_definitions(cgv_math PRIVATE CGV_MATH_EXPORTS)

target_include_directories(cgv_math PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<INSTALL_INTERFACE:include>)

install(TARGETS cgv_math EXPORT cgv DESTINATION ${CGV_LIB_DEST})
