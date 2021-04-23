
add_library(crg_grid grid.cxx main.cxx)
target_link_libraries(crg_grid
        cgv_render
        cgv_gui
        cgv_base
        cgv_type
        cgv_reflect
        cgv_utils
        cgv_signal
        cgv_data
        cgv_gl)
target_compile_definitions(crg_grid PRIVATE _USRDLL)

install(TARGETS crg_grid EXPORT cgv_plugins DESTINATION ${CGV_LIB_DEST})
