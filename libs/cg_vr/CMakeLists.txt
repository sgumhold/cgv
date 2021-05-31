
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(cg_vr ${SOURCES})
target_compile_definitions(cg_vr PRIVATE CG_VR_EXPORTS)
target_link_libraries(cg_vr
        cgv_utils cgv_type cgv_reflect cgv_reflect_types cgv_data
        cgv_signal cgv_os cgv_base cgv_math cgv_media cgv_gui
        vr)

install(TARGETS cg_vr EXPORT cgv_libs DESTINATION ${CGV_LIB_DEST})
