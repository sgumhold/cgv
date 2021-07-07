
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

# Shared library
add_library(cg_fltk ${SOURCES})
target_link_libraries(cg_fltk
        fltk
        cgv_gl
        cgv_gui
        cgv_render
        cgv_media
        cgv_utils
        cgv_os
        cgv_base
        cgv_data
        cgv_type
        cgv_signal
        cgv_reflect)
target_compile_definitions(cg_fltk PRIVATE CG_FLTK_EXPORTS CGV_GUI_FLTK_EXPORTS)

install(TARGETS cg_fltk EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})

# Static library
add_library(cg_fltk_static STATIC ${SOURCES})
target_link_libraries(cg_fltk_static
        fltk_static
        cgv_gl_static
        cgv_gui_static
        cgv_render_static
        cgv_media_static
        cgv_utils_static
        cgv_os_static
        cgv_base_static
        cgv_data_static
        cgv_type_static
        cgv_signal_static
        cgv_reflect_static)
target_compile_definitions(cg_fltk_static PRIVATE CGV_GUI_FLTK_FORCE_STATIC)

install(TARGETS cg_fltk_static EXPORT cgv_plugins DESTINATION ${CGV_BIN_DEST})
