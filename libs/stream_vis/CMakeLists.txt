
file(GLOB_RECURSE SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "*.cxx")

add_library(stream_vis ${SOURCES})
target_compile_definitions(stream_vis PRIVATE STREAM_VIS_EXPORTS)
target_link_libraries(stream_vis
        cgv_utils 
        cgv_type 
        cgv_reflect 
        cgv_data 
        cgv_signal 
        cgv_base 
        cgv_media 
        cgv_gui 
        cgv_render
        cgv_os 
        cgv_gl 
        glew
        plot)
target_include_directories(stream_vis PUBLIC
        $<BUILD_INTERFACE:${CGV_DIR}/libs>
        $<INSTALL_INTERFACE:${CGV_LIBS_INCLUDE_DEST}>)

install(TARGETS stream_vis EXPORT cgv_libs DESTINATION ${CGV_BIN_DEST})
