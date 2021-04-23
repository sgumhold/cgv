
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_os ${SOURCES})
target_compile_definitions(cgv_os PRIVATE CGV_OS_EXPORTS)
target_link_libraries(cgv_os PUBLIC cgv_utils)

if (WIN32)
    # In windows we also need ws2_32
    target_link_libraries(cgv_os PUBLIC ws2_32)
else ()
    target_link_libraries(cgv_os PUBLIC pthread)
endif ()

target_include_directories(cgv_os PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}>
        $<INSTALL_INTERFACE:include>)

install(TARGETS cgv_os EXPORT cgv DESTINATION ${CGV_LIB_DEST})
