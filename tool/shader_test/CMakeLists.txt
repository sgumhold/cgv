include(shader_test.cmake)

ppp_compile(${CGV_DIR} PPP_FILES PPP_INCLUDES PPP_INSTALL_DIR
        ../../libs/cgv_gl/gl/gl_context_switch.ph
        ../../cgv/type/traits/method_pointer.ph
        ../../cgv/type/invalid_type.ph
        ../../cgv/signal/signal.ph
        ../../cgv/signal/abst_signal.ph
        ../../cgv/signal/bool_signal.ph
        ../../cgv/signal/rebind.ph
        ../../cgv/utils/ostream_printf.ph)

file(GLOB SOURCES_UTILS ${CGV_DIR}/cgv/utils/*.cxx)
file(GLOB_RECURSE SOURCES_TYPE ${CGV_DIR}/cgv/type/*.cxx)
file(GLOB SOURCES_REFLECT ${CGV_DIR}/cgv/reflect/*.cxx)
file(GLOB SOURCES_DATA ${CGV_DIR}/cgv/data/*.cxx)
file(GLOB SOURCES_BASE ${CGV_DIR}/cgv/base/*.cxx)
file(GLOB SOURCES_SIGNAL ${CGV_DIR}/cgv/signal/*.cxx)
file(GLOB SOURCES_PPP ${CGV_DIR}/cgv/ppp/*.cxx)

add_executable(shader_test
        shader_test.cxx
        ${CGV_DIR}/cgv/gui/file_dialog.cxx
        ${CGV_DIR}/cgv/gui/gui_driver.cxx
        ${CGV_DIR}/cgv/gui/control.cxx
        ${CGV_DIR}/cgv/media/image/image_writer.cxx
        ${CGV_DIR}/cgv/media/font/font.cxx
        ${CGV_DIR}/cgv/media/illum/phong_material.cxx
        ${CGV_DIR}/cgv/media/illum/surface_material.cxx
        ${CGV_DIR}/cgv/media/illum/light_source.cxx
        ${CGV_DIR}/cgv/media/font/font_server.cxx
        ${CGV_DIR}/cgv/os/clipboard.cxx
        ${CGV_DIR}/cgv/render/context.cxx
        ${CGV_DIR}/cgv/render/drawable.cxx
        ${CGV_DIR}/libs/cgv_gl/gl/gl.cxx
        ${CGV_DIR}/libs/cgv_gl/gl/gl_texture_tools.cxx
        ${CGV_DIR}/libs/cgv_gl/gl/gl_context.cxx
        ${CGV_DIR}/libs/cgv_gl/gl/gl_context_win32.cxx
        ${CGV_DIR}/cgv/render/shader_code.cxx
        ${CGV_DIR}/cgv/render/frame_buffer.cxx
        ${CGV_DIR}/cgv/render/vertex_buffer.cxx
        ${CGV_DIR}/cgv/render/attribute_array_binding.cxx
        ${CGV_DIR}/cgv/render/render_buffer.cxx
        ${CGV_DIR}/cgv/render/view.cxx
        ${CGV_DIR}/cgv/render/shader_program.cxx
        ${SOURCES_UTILS}
        ${SOURCES_TYPE}
        ${SOURCES_REFLECT}
        ${SOURCES_DATA}
        ${SOURCES_BASE}
        ${SOURCES_SIGNAL}
        ${SOURCES_PPP}
        ${PPP_FILES})
target_link_libraries(shader_test
        fltk
        glew
        ${X11_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${CMAKE_DL_LIBS})
target_include_directories(shader_test PRIVATE
        ${CGV_DIR}
        ${CGV_DIR}/libs
        ${PPP_INCLUDES})
target_compile_definitions(shader_test PRIVATE CGV_FORCE_STATIC)
set_target_properties(shader_test PROPERTIES CGVPROP_TYPE "app")
set_target_properties(shader_test PROPERTIES FOLDER "App/tool")

install(TARGETS shader_test EXPORT cgv_tools DESTINATION ${CGV_BIN_DEST})
