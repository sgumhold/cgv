
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(cgv_os CORE_LIB SOURCES ${SOURCES} DEPENDENCIES cgv_utils)

# we need C++17 for std::data in the named pipes sublib (pipe.hpp)
target_compile_features(cgv_os PRIVATE cxx_std_17)
target_compile_features(cgv_os_static PRIVATE cxx_std_17)
set_target_properties(cgv_os cgv_os_static PROPERTIES CXX_STANDARD 17)
set_target_properties(cgv_os cgv_os_static  PROPERTIES CXX_STANDARD_REQUIRED ON)

if (WIN32)
    # in windows we also need ws2_32
    target_link_libraries(cgv_os PUBLIC ws2_32)
    target_link_libraries(cgv_os_static PUBLIC ws2_32)
else ()
    # in all other cases we need pthreads
    target_link_libraries(cgv_os PUBLIC pthread)
    target_link_libraries(cgv_os_static PUBLIC pthread)
endif ()
