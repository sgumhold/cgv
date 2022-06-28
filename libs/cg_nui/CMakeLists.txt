
file(GLOB SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

cgv_create_lib(cg_nui SOURCES ${SOURCES}
        DEPENDENCIES cgv_utils cgv_type cgv_reflect cgv_data cgv_base cgv_media cgv_render cgv_gl vr cg_vr cgv_os cgv_gui rect_pack screen_capture_lite)

target_compile_definitions(cg_nui PRIVATE CG_NUI_EXPORTS)
