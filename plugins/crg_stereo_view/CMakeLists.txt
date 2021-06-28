
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

