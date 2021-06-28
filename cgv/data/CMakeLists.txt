
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_data ${SOURCES})
target_compile_definitions(cgv_data PRIVATE CGV_DATA_EXPORTS)
target_link_libraries(cgv_data PUBLIC cgv_utils cgv_type cgv_reflect)

target_include_directories(cgv_data PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<INSTALL_INTERFACE:include>)

install(TARGETS cgv_data EXPORT cgv DESTINATION ${CGV_BIN_DEST})
