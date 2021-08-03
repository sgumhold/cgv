
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(cgv_os CORE_LIB SOURCES ${SOURCES} DEPENDENCIES cgv_utils)
if (WIN32)
    # In windows we also need ws2_32
    target_link_libraries(cgv_os PUBLIC ws2_32)
    target_link_libraries(cgv_os_static PUBLIC ws2_32)
else ()
    target_link_libraries(cgv_os PUBLIC pthread)
    target_link_libraries(cgv_os_static PUBLIC pthread)
endif ()
