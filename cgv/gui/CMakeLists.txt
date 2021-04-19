
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cgv_gui ${SOURCES})
target_compile_definitions(cgv_gui PRIVATE CGV_GUI_EXPORTS)
target_include_directories(cgv_gui PUBLIC ${CGV_DIR})
target_link_libraries(cgv_gui PUBLIC
        cgv_utils
        cgv_type
        cgv_reflect
        cgv_data
        cgv_base
        cgv_media
        cgv_signal)
