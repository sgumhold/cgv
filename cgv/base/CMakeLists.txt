
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

ppp_compile("${CGV_DIR}" PPP_FILES PPP_INCLUDES
        ../../cgv/type/traits/method_pointer.ph
        ../../cgv/base/call_interface_impl.ph)

add_library(cgv_base ${SOURCES})
target_compile_definitions(cgv_base PRIVATE CGV_BASE_EXPORTS)
target_include_directories(cgv_base PUBLIC
        ${CGV_DIR}
        ${PPP_INCLUDES})
target_link_libraries(cgv_base PUBLIC
        cgv_utils
        cgv_type
        cgv_data
        cgv_reflect
        cgv_signal
        ${CMAKE_DL_LIBS})
