
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")
file(GLOB_RECURSE SHADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "glsl/*.gl*")

add_library(plot ${SOURCES} ${SHADERS})
target_compile_definitions(plot PRIVATE PLOT_EXPORTS)
target_link_libraries(plot
        cgv_gl
        cgv_utils
        cgv_type
        cgv_reflect
        cgv_data
        cgv_base
        cgv_media
        cgv_os
        cgv_gui
        cgv_render)
target_include_directories(plot PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS plot EXPORT plot DESTINATION ${CGV_LIB_DEST})
install(EXPORT plot DESTINATION ${CGV_LIB_DEST})
install(DIRECTORY glsl/. DESTINATION ${CGV_LIBS_INCLUDE_DEST}/plot/glsl)
