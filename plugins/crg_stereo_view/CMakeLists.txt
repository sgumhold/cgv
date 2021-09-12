
add_library(crg_stereo_view stereo_view_interactor.cxx)
target_link_libraries(crg_stereo_view
        cgv_render
        cgv_gui
        cgv_utils
        cgv_type
        cgv_reflect
        cgv_data
        cgv_base
        cgv_signal
        cgv_gl
        glsu)
target_compile_definitions(crg_stereo_view PRIVATE CGV_RENDER_STEREO_VIEW_EXPORTS)
install(TARGETS crg_stereo_view EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})

add_library(crg_stereo_view_lib stereo_view_interactor.cxx)
target_link_libraries(crg_stereo_view_lib
        cgv_render
        cgv_gui
        cgv_utils
        cgv_type
        cgv_reflect
        cgv_data
        cgv_base
        cgv_signal
        cgv_gl
        glsu)
target_compile_definitions(crg_stereo_view_lib PRIVATE CGV_RENDER_STEREO_VIEW_EXPORTS NO_STEREO_VIEW_INTERACTOR)
install(TARGETS crg_stereo_view_lib EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})

# Static Library
add_library(crg_stereo_view_static STATIC stereo_view_interactor.cxx)
target_link_libraries(crg_stereo_view_static
        cgv_render_static
        cgv_gui_static
        cgv_utils_static
        cgv_type_static
        cgv_reflect_static
        cgv_data_static
        cgv_base_static
        cgv_signal_static
        cgv_gl_static
        glsu_static)
target_compile_definitions(crg_stereo_view_static PRIVATE CGV_FORCE_STATIC CGV_GUI_FORCE_STATIC)
install(TARGETS crg_stereo_view_static EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
